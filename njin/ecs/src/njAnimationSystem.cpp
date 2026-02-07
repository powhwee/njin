#include "ecs/njAnimationSystem.h"

#include <iostream>
#include <vector>

#include "core/njAnimation.h"
#include "ecs/Components.h"
#include "math/njQuat.h"
#include "math/njVec3.h"

#include <cmath>

namespace njin::ecs {

    namespace {
        using namespace njin::math;
        using namespace njin::core;

        template<typename T>
        T interpolate_linear(const T& a, const T& b, float t) {
            return a + (b - a) * t;
        }

        // Helper to find the interval [t1, t2] such that t1 <= time <= t2
        size_t find_keyframe_index(const std::vector<njKeyframe>& keyframes,
                                   float time) {
            if (keyframes.empty())
                return 0;
            if (time <= keyframes.front().time)
                return 0;
            if (time >= keyframes.back().time)
                return keyframes.size() - 1;

            for (size_t i = 0; i < keyframes.size() - 1; ++i) {
                if (time >= keyframes[i].time && time < keyframes[i + 1].time) {
                    return i;
                }
            }
            return keyframes.size() - 1;
        }
    }  // namespace

    njAnimationSystem::njAnimationSystem(float& delta_time) :
        njSystem(TickGroup::One),
        delta_time_{ delta_time } {}

    void njAnimationSystem::update(const njEntityManager& entity_manager) {
        auto view =
        entity_manager.get_views<njNodeComponent, njAnimationComponent>();

        static int debug_counter = 0;
        if (debug_counter++ % 300 == 0) {
            std::cout << "[AnimSystem] entities with "
                         "NodeComp+AnimComp: "
                      << view.size() << std::endl;
        }

        for (auto [entity_id, components] : view) {
            auto node_comp = std::get<0>(components);
            auto anim_comp = std::get<1>(components);

            if (!anim_comp->playing || !node_comp->skeleton ||
                !anim_comp->animations)
                continue;

            // 1. Advance time
            anim_comp->current_time += delta_time_ * anim_comp->speed;

            // 2. Find current animation
            const njAnimation* current_anim = nullptr;
            for (const auto& anim : *anim_comp->animations) {
                if (anim.name == anim_comp->current_animation) {
                    current_anim = &anim;
                    break;
                }
            }

            if (!current_anim) {
                if (debug_counter % 300 == 1) {
                    std::cout << "[AnimSystem] No match for '"
                              << anim_comp->current_animation << "' in "
                              << anim_comp->animations->size() << " animations"
                              << std::endl;
                    for (const auto& a : *anim_comp->animations) {
                        std::cout
                        << "  - '" << a.name << "' (dur=" << a.duration
                        << ", ch=" << a.channels.size() << ")" << std::endl;
                    }
                }
                continue;
            }

            // Handle looping
            if (anim_comp->current_time > current_anim->duration) {
                if (anim_comp->loop) {
                    anim_comp->current_time = std::fmod(anim_comp->current_time,
                                                        current_anim->duration);
                    // Handle edge case where duration is 0
                    if (current_anim->duration <= 0.0001f)
                        anim_comp->current_time = 0;
                } else {
                    anim_comp->current_time = current_anim->duration;
                    anim_comp->playing = false;
                }
            }

            // 3. Prepare node transforms
            size_t node_count = node_comp->skeleton->nodes.size();
            if (anim_comp->pose.size() != node_count) {
                anim_comp->pose.resize(node_count, njMat4f::Identity());
            }

            // Struct to hold blended TRS for each node
            struct TRS {
                njVec3f t;
                njQuatf r;
                njVec3f s;
                // We don't strictly need dirty flags if we initialize with bind pose
            };

            std::vector<TRS> blended_trs(node_count);

            // Initialize with bind pose
            for (size_t i = 0; i < node_count; ++i) {
                blended_trs[i].t =
                node_comp->skeleton->nodes[i].bind_translation;
                blended_trs[i].r = node_comp->skeleton->nodes[i].bind_rotation;
                blended_trs[i].s = node_comp->skeleton->nodes[i].bind_scale;
            }

            // 4. Sample Animation Channels
            for (const auto& channel : current_anim->channels) {
                int idx = channel.target_node;
                if (idx < 0 || idx >= node_count)
                    continue;

                // Sample keyframes
                const auto& keyframes = channel.keyframes;
                if (keyframes.empty())
                    continue;

                size_t k0_idx = find_keyframe_index(keyframes,
                                                    anim_comp->current_time);
                size_t k1_idx = k0_idx + 1;
                if (k1_idx >= keyframes.size())
                    k1_idx = k0_idx;

                const auto& k0 = keyframes[k0_idx];
                const auto& k1 = keyframes[k1_idx];

                float t = 0.0f;
                if (k1_idx != k0_idx) {
                    float duration = k1.time - k0.time;
                    if (duration > 0.0001f) {
                        t = (anim_comp->current_time - k0.time) / duration;
                    }
                }

                // Interpolate
                if (channel.path == njAnimationChannel::Path::Translation) {
                    njVec3f val0 = std::get<njVec3f>(k0.value);
                    njVec3f val1 = std::get<njVec3f>(k1.value);
                    blended_trs[idx].t = interpolate_linear(val0, val1, t);
                } else if (channel.path == njAnimationChannel::Path::Scale) {
                    njVec3f val0 = std::get<njVec3f>(k0.value);
                    njVec3f val1 = std::get<njVec3f>(k1.value);
                    blended_trs[idx].s = interpolate_linear(val0, val1, t);
                } else if (channel.path == njAnimationChannel::Path::Rotation) {
                    njQuatf val0 = std::get<njQuatf>(k0.value);
                    njQuatf val1 = std::get<njQuatf>(k1.value);
                    blended_trs[idx].r = slerp(val0, val1, t);
                }
            }

            // 5. Compute Global Transforms (Runtime Hierarchy)
            auto& nodes = node_comp->skeleton->nodes;

            struct Recursor {
                const std::vector<TRS>& trs_vals;
                const std::vector<njSkeletonNode>& nodes;
                njAnimationComponent* anim_comp;

                void traverse(int node_idx, const njMat4f& parent_transform) {
                    // Compute local matrix
                    const auto& trs = trs_vals[node_idx];
                    njMat4f T_mat(njMat4Type::Translation, trs.t);
                    njMat4f R_mat(trs.r);
                    njMat4f S_mat(njMat4Type::Scale, trs.s);

                    // Order: T * R * S
                    njMat4f local_mat = T_mat * R_mat * S_mat;
                    njMat4f global_mat = parent_transform * local_mat;

                    anim_comp->pose[node_idx] = global_mat;

                    for (int child : nodes[node_idx].children) {
                        traverse(child, global_mat);
                    }
                }
            };

            Recursor recursor{ blended_trs, nodes, anim_comp };

            const auto& root_nodes = node_comp->skeleton->root_nodes;
            for (int root : root_nodes) {
                recursor.traverse(root, njMat4f::Identity());
            }

            // Compute joint matrices for GPU skinning
            const auto& joint_nodes = node_comp->skeleton->joint_nodes;
            const auto& ibms = node_comp->skeleton->inverse_bind_matrices;
            if (!joint_nodes.empty() && joint_nodes.size() == ibms.size()) {
                anim_comp->joint_matrices.resize(joint_nodes.size());
                for (size_t j = 0; j < joint_nodes.size(); ++j) {
                    int node_idx = joint_nodes[j];
                    if (node_idx >= 0 && node_idx < anim_comp->pose.size()) {
                        anim_comp->joint_matrices[j] =
                        anim_comp->pose[node_idx] * ibms[j];
                    } else {
                        anim_comp->joint_matrices[j] = njMat4f::Identity();
                    }
                }
            }
        }
    }
}  // namespace njin::ecs
