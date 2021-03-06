/*************************************************************************/
/*  rasterizer_effects_rd.h                                              */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2020 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2020 Godot Engine contributors (cf. AUTHORS.md).   */
/*                                                                       */
/* Permission is hereby granted, free of charge, to any person obtaining */
/* a copy of this software and associated documentation files (the       */
/* "Software"), to deal in the Software without restriction, including   */
/* without limitation the rights to use, copy, modify, merge, publish,   */
/* distribute, sublicense, and/or sell copies of the Software, and to    */
/* permit persons to whom the Software is furnished to do so, subject to */
/* the following conditions:                                             */
/*                                                                       */
/* The above copyright notice and this permission notice shall be        */
/* included in all copies or substantial portions of the Software.       */
/*                                                                       */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.*/
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                */
/*************************************************************************/

#ifndef RASTERIZER_EFFECTS_RD_H
#define RASTERIZER_EFFECTS_RD_H

#include "core/math/camera_matrix.h"
#include "servers/rendering/rasterizer_rd/render_pipeline_vertex_format_cache_rd.h"
#include "servers/rendering/rasterizer_rd/shaders/blur.glsl.gen.h"
#include "servers/rendering/rasterizer_rd/shaders/bokeh_dof.glsl.gen.h"
#include "servers/rendering/rasterizer_rd/shaders/copy.glsl.gen.h"
#include "servers/rendering/rasterizer_rd/shaders/cubemap_downsampler.glsl.gen.h"
#include "servers/rendering/rasterizer_rd/shaders/cubemap_filter.glsl.gen.h"
#include "servers/rendering/rasterizer_rd/shaders/cubemap_roughness.glsl.gen.h"
#include "servers/rendering/rasterizer_rd/shaders/luminance_reduce.glsl.gen.h"
#include "servers/rendering/rasterizer_rd/shaders/roughness_limiter.glsl.gen.h"
#include "servers/rendering/rasterizer_rd/shaders/screen_space_reflection.glsl.gen.h"
#include "servers/rendering/rasterizer_rd/shaders/screen_space_reflection_filter.glsl.gen.h"
#include "servers/rendering/rasterizer_rd/shaders/screen_space_reflection_scale.glsl.gen.h"
#include "servers/rendering/rasterizer_rd/shaders/specular_merge.glsl.gen.h"
#include "servers/rendering/rasterizer_rd/shaders/ssao.glsl.gen.h"
#include "servers/rendering/rasterizer_rd/shaders/ssao_blur.glsl.gen.h"
#include "servers/rendering/rasterizer_rd/shaders/ssao_minify.glsl.gen.h"
#include "servers/rendering/rasterizer_rd/shaders/subsurface_scattering.glsl.gen.h"
#include "servers/rendering/rasterizer_rd/shaders/tonemap.glsl.gen.h"

#include "servers/rendering_server.h"

class RasterizerEffectsRD {

	enum BlurMode {
		BLUR_MODE_GAUSSIAN_BLUR,
		BLUR_MODE_GAUSSIAN_GLOW,
		BLUR_MODE_GAUSSIAN_GLOW_AUTO_EXPOSURE,
		BLUR_MODE_DOF_NEAR_LOW,
		BLUR_MODE_DOF_NEAR_MEDIUM,
		BLUR_MODE_DOF_NEAR_HIGH,
		BLUR_MODE_DOF_NEAR_MERGE_LOW,
		BLUR_MODE_DOF_NEAR_MERGE_MEDIUM,
		BLUR_MODE_DOF_NEAR_MERGE_HIGH,
		BLUR_MODE_DOF_FAR_LOW,
		BLUR_MODE_DOF_FAR_MEDIUM,
		BLUR_MODE_DOF_FAR_HIGH,
		BLUR_MODE_SSAO_MERGE,
		BLUR_MODE_SIMPLY_COPY,
		BLUR_MODE_MIPMAP,
		BLUR_MODE_LINEARIZE_DEPTH,
		BLUR_MODE_MAX,

	};

	enum {
		BLUR_FLAG_HORIZONTAL = (1 << 0),
		BLUR_FLAG_USE_BLUR_SECTION = (1 << 1),
		BLUR_FLAG_USE_ORTHOGONAL_PROJECTION = (1 << 2),
		BLUR_FLAG_DOF_NEAR_FIRST_TAP = (1 << 3),
		BLUR_FLAG_GLOW_FIRST_PASS = (1 << 4),
		BLUR_FLAG_FLIP_Y = (1 << 5),
		BLUR_COPY_FORCE_LUMINANCE = (1 << 6)
	};

	struct BlurPushConstant {
		float section[4];
		float pixel_size[2];
		uint32_t flags;
		uint32_t pad;
		//glow
		float glow_strength;
		float glow_bloom;
		float glow_hdr_threshold;
		float glow_hdr_scale;
		float glow_exposure;
		float glow_white;
		float glow_luminance_cap;
		float glow_auto_exposure_grey;
		//dof
		float dof_begin;
		float dof_end;
		float dof_radius;
		float dof_pad;

		float dof_dir[2];
		float camera_z_far;
		float camera_z_near;

		float ssao_color[4];
	};

	struct Blur {
		BlurPushConstant push_constant;
		BlurShaderRD shader;
		RID shader_version;
		RenderPipelineVertexFormatCacheRD pipelines[BLUR_MODE_MAX];

	} blur;

	struct CubemapRoughnessPushConstant {
		uint32_t face_id;
		uint32_t sample_count;
		float roughness;
		uint32_t use_direct_write;
		float face_size;
		float pad[3];
	};

	struct CubemapRoughness {

		CubemapRoughnessPushConstant push_constant;
		CubemapRoughnessShaderRD shader;
		RID shader_version;
		RID pipeline;
	} roughness;

	enum TonemapMode {
		TONEMAP_MODE_NORMAL,
		TONEMAP_MODE_BICUBIC_GLOW_FILTER,
		TONEMAP_MODE_MAX
	};

	struct TonemapPushConstant {
		float bcs[3];
		uint32_t use_bcs;

		uint32_t use_glow;
		uint32_t use_auto_exposure;
		uint32_t use_color_correction;
		uint32_t tonemapper;

		uint32_t glow_texture_size[2];

		float glow_intensity;
		uint32_t glow_level_flags;
		uint32_t glow_mode;

		float exposure;
		float white;
		float auto_exposure_grey;
	};

	struct Tonemap {

		TonemapPushConstant push_constant;
		TonemapShaderRD shader;
		RID shader_version;
		RenderPipelineVertexFormatCacheRD pipelines[TONEMAP_MODE_MAX];
	} tonemap;

	enum LuminanceReduceMode {
		LUMINANCE_REDUCE_READ,
		LUMINANCE_REDUCE,
		LUMINANCE_REDUCE_WRITE,
		LUMINANCE_REDUCE_MAX
	};

	struct LuminanceReducePushConstant {
		int32_t source_size[2];
		float max_luminance;
		float min_luminance;
		float exposure_adjust;
		float pad[3];
	};

	struct LuminanceReduce {

		LuminanceReducePushConstant push_constant;
		LuminanceReduceShaderRD shader;
		RID shader_version;
		RID pipelines[LUMINANCE_REDUCE_MAX];
	} luminance_reduce;

	struct CopyToDPPushConstant {
		float bias;
		float z_far;
		float z_near;
		uint32_t z_flip;
	};

	enum CopyMode {
		COPY_MODE_CUBE_TO_DP,
		COPY_MODE_MAX
	};

	struct Copy {

		CopyShaderRD shader;
		RID shader_version;
		RenderPipelineVertexFormatCacheRD pipelines[COPY_MODE_MAX];
	} copy;

	struct BokehPushConstant {
		uint32_t size[2];
		float z_far;
		float z_near;

		uint32_t orthogonal;
		float blur_size;
		float blur_scale;
		uint32_t steps;

		uint32_t blur_near_active;
		float blur_near_begin;
		float blur_near_end;
		uint32_t blur_far_active;

		float blur_far_begin;
		float blur_far_end;
		uint32_t second_pass;
		uint32_t half_size;

		uint32_t use_jitter;
		float jitter_seed;
		uint32_t pad[2];
	};

	enum BokehMode {
		BOKEH_GEN_BLUR_SIZE,
		BOKEH_GEN_BOKEH_BOX,
		BOKEH_GEN_BOKEH_HEXAGONAL,
		BOKEH_GEN_BOKEH_CIRCULAR,
		BOKEH_COMPOSITE,
		BOKEH_MAX
	};

	struct Bokeh {

		BokehPushConstant push_constant;
		BokehDofShaderRD shader;
		RID shader_version;
		RID pipelines[BOKEH_MAX];
	} bokeh;

	enum SSAOMode {
		SSAO_MINIFY_FIRST,
		SSAO_MINIFY_MIPMAP,
		SSAO_GATHER_LOW,
		SSAO_GATHER_MEDIUM,
		SSAO_GATHER_HIGH,
		SSAO_GATHER_ULTRA,
		SSAO_GATHER_LOW_HALF,
		SSAO_GATHER_MEDIUM_HALF,
		SSAO_GATHER_HIGH_HALF,
		SSAO_GATHER_ULTRA_HALF,
		SSAO_BLUR_PASS,
		SSAO_BLUR_PASS_HALF,
		SSAO_BLUR_UPSCALE,
		SSAO_MAX
	};

	struct SSAOMinifyPushConstant {
		float pixel_size[2];
		float z_far;
		float z_near;
		int32_t source_size[2];
		uint32_t orthogonal;
		uint32_t pad;
	};

	struct SSAOGatherPushConstant {
		int32_t screen_size[2];
		float z_far;
		float z_near;

		uint32_t orthogonal;
		float intensity_div_r6;
		float radius;
		float bias;

		float proj_info[4];
		float pixel_size[2];
		float proj_scale;
		uint32_t pad;
	};

	struct SSAOBlurPushConstant {
		float edge_sharpness;
		int32_t filter_scale;
		float z_far;
		float z_near;
		uint32_t orthogonal;
		uint32_t pad[3];
		int32_t axis[2];
		int32_t screen_size[2];
	};

	struct SSAO {

		SSAOMinifyPushConstant minify_push_constant;
		SsaoMinifyShaderRD minify_shader;
		RID minify_shader_version;

		SSAOGatherPushConstant gather_push_constant;
		SsaoShaderRD gather_shader;
		RID gather_shader_version;

		SSAOBlurPushConstant blur_push_constant;
		SsaoBlurShaderRD blur_shader;
		RID blur_shader_version;

		RID pipelines[SSAO_MAX];
	} ssao;

	struct RoughnessLimiterPushConstant {
		int32_t screen_size[2];
		float curve;
		uint32_t pad;
	};

	struct RoughnessLimiter {

		RoughnessLimiterPushConstant push_constant;
		RoughnessLimiterShaderRD shader;
		RID shader_version;
		RID pipeline;

	} roughness_limiter;

	struct CubemapDownsamplerPushConstant {
		uint32_t face_size;
		float pad[3];
	};

	struct CubemapDownsampler {

		CubemapDownsamplerPushConstant push_constant;
		CubemapDownsamplerShaderRD shader;
		RID shader_version;
		RID pipeline;

	} cubemap_downsampler;

	enum CubemapFilterMode {
		FILTER_MODE_HIGH_QUALITY,
		FILTER_MODE_LOW_QUALITY,
		FILTER_MODE_HIGH_QUALITY_ARRAY,
		FILTER_MODE_LOW_QUALITY_ARRAY,
		FILTER_MODE_MAX,
	};

	struct CubemapFilter {

		CubemapFilterShaderRD shader;
		RID shader_version;
		RID pipelines[FILTER_MODE_MAX];
		RID uniform_set;
		RID image_uniform_set;
		RID coefficient_buffer;
		bool use_high_quality;

	} filter;

	struct SkyPushConstant {
		float orientation[12];
		float proj[4];
		float position[3];
		float multiplier;
		float time;
		float pad[3];
	};

	enum SpecularMergeMode {
		SPECULAR_MERGE_ADD,
		SPECULAR_MERGE_SSR,
		SPECULAR_MERGE_ADDITIVE_ADD,
		SPECULAR_MERGE_ADDITIVE_SSR,
		SPECULAR_MERGE_MAX
	};

	struct SpecularMerge {

		SpecularMergeShaderRD shader;
		RID shader_version;
		RenderPipelineVertexFormatCacheRD pipelines[SPECULAR_MERGE_MAX];

	} specular_merge;

	enum ScreenSpaceReflectionMode {
		SCREEN_SPACE_REFLECTION_NORMAL,
		SCREEN_SPACE_REFLECTION_ROUGH,
		SCREEN_SPACE_REFLECTION_MAX,
	};

	struct ScreenSpaceReflectionPushConstant {

		float proj_info[4];

		int32_t screen_size[2];
		float camera_z_near;
		float camera_z_far;

		int32_t num_steps;
		float depth_tolerance;
		float distance_fade;
		float curve_fade_in;

		uint32_t orthogonal;
		float filter_mipmap_levels;
		uint32_t use_half_res;
		uint8_t metallic_mask[4];

		float projection[16];
	};

	struct ScreenSpaceReflection {

		ScreenSpaceReflectionPushConstant push_constant;
		ScreenSpaceReflectionShaderRD shader;
		RID shader_version;
		RID pipelines[SCREEN_SPACE_REFLECTION_MAX];

	} ssr;

	struct ScreenSpaceReflectionFilterPushConstant {

		float proj_info[4];

		uint32_t orthogonal;
		float edge_tolerance;
		int32_t increment;
		uint32_t pad;

		int32_t screen_size[2];
		uint32_t vertical;
		uint32_t steps;
	};
	enum {
		SCREEN_SPACE_REFLECTION_FILTER_HORIZONTAL,
		SCREEN_SPACE_REFLECTION_FILTER_VERTICAL,
		SCREEN_SPACE_REFLECTION_FILTER_MAX,
	};

	struct ScreenSpaceReflectionFilter {

		ScreenSpaceReflectionFilterPushConstant push_constant;
		ScreenSpaceReflectionFilterShaderRD shader;
		RID shader_version;
		RID pipelines[SCREEN_SPACE_REFLECTION_FILTER_MAX];
	} ssr_filter;

	struct ScreenSpaceReflectionScalePushConstant {

		int32_t screen_size[2];
		float camera_z_near;
		float camera_z_far;

		uint32_t orthogonal;
		uint32_t filter;
		uint32_t pad[2];
	};

	struct ScreenSpaceReflectionScale {

		ScreenSpaceReflectionScalePushConstant push_constant;
		ScreenSpaceReflectionScaleShaderRD shader;
		RID shader_version;
		RID pipeline;
	} ssr_scale;

	struct SubSurfaceScatteringPushConstant {

		int32_t screen_size[2];
		float camera_z_far;
		float camera_z_near;

		uint32_t vertical;
		uint32_t orthogonal;
		float unit_size;
		float scale;

		float depth_scale;
		uint32_t pad[3];
	};

	struct SubSurfaceScattering {

		SubSurfaceScatteringPushConstant push_constant;
		SubsurfaceScatteringShaderRD shader;
		RID shader_version;
		RID pipelines[3]; //3 quality levels
	} sss;

	RID default_sampler;
	RID default_mipmap_sampler;
	RID index_buffer;
	RID index_array;

	Map<RID, RID> texture_to_uniform_set_cache;

	Map<RID, RID> image_to_uniform_set_cache;

	struct TexturePair {
		RID texture1;
		RID texture2;
		_FORCE_INLINE_ bool operator<(const TexturePair &p_pair) const {
			if (texture1 == p_pair.texture1) {
				return texture2 < p_pair.texture2;
			} else {
				return texture1 < p_pair.texture1;
			}
		}
	};

	Map<RID, RID> texture_to_compute_uniform_set_cache;
	Map<TexturePair, RID> texture_pair_to_compute_uniform_set_cache;
	Map<TexturePair, RID> image_pair_to_compute_uniform_set_cache;

	RID _get_uniform_set_from_image(RID p_texture);
	RID _get_uniform_set_from_texture(RID p_texture, bool p_use_mipmaps = false);
	RID _get_compute_uniform_set_from_texture(RID p_texture, bool p_use_mipmaps = false);
	RID _get_compute_uniform_set_from_texture_pair(RID p_texture, RID p_texture2, bool p_use_mipmaps = false);
	RID _get_compute_uniform_set_from_image_pair(RID p_texture, RID p_texture2);

public:
	//TODO must re-do most of the shaders in compute

	void region_copy(RID p_source_rd_texture, RID p_dest_framebuffer, const Rect2 &p_region);
	void copy_to_rect(RID p_source_rd_texture, RID p_dest_framebuffer, const Rect2 &p_rect, bool p_flip_y = false, bool p_force_luminance = false);
	void copy_to_rect_and_linearize(RID p_source_rd_texture, RID p_dest_framebuffer, const Rect2 &p_rect, bool p_flip_y, float p_z_near, float p_z_far);
	void gaussian_blur(RID p_source_rd_texture, RID p_framebuffer_half, RID p_rd_texture_half, RID p_dest_framebuffer, const Vector2 &p_pixel_size, const Rect2 &p_region);
	void gaussian_glow(RID p_source_rd_texture, RID p_framebuffer_half, RID p_rd_texture_half, RID p_dest_framebuffer, const Vector2 &p_pixel_size, float p_strength = 1.0, bool p_first_pass = false, float p_luminance_cap = 16.0, float p_exposure = 1.0, float p_bloom = 0.0, float p_hdr_bleed_treshold = 1.0, float p_hdr_bleed_scale = 1.0, RID p_auto_exposure = RID(), float p_auto_exposure_grey = 1.0);

	void cubemap_roughness(RID p_source_rd_texture, RID p_dest_framebuffer, uint32_t p_face_id, uint32_t p_sample_count, float p_roughness, float p_size);
	void make_mipmap(RID p_source_rd_texture, RID p_framebuffer_half, const Vector2 &p_pixel_size);
	void copy_cubemap_to_dp(RID p_source_rd_texture, RID p_dest_framebuffer, const Rect2 &p_rect, float p_z_near, float p_z_far, float p_bias, bool p_dp_flip);
	void luminance_reduction(RID p_source_texture, const Size2i p_source_size, const Vector<RID> p_reduce, RID p_prev_luminance, float p_min_luminance, float p_max_luminance, float p_adjust, bool p_set = false);
	void bokeh_dof(RID p_base_texture, RID p_depth_texture, const Size2i &p_base_texture_size, RID p_secondary_texture, RID p_bokeh_texture1, RID p_bokeh_texture2, bool p_dof_far, float p_dof_far_begin, float p_dof_far_size, bool p_dof_near, float p_dof_near_begin, float p_dof_near_size, float p_bokeh_size, RS::DOFBokehShape p_bokeh_shape, RS::DOFBlurQuality p_quality, bool p_use_jitter, float p_cam_znear, float p_cam_zfar, bool p_cam_orthogonal);

	struct TonemapSettings {

		bool use_glow = false;
		enum GlowMode {
			GLOW_MODE_ADD,
			GLOW_MODE_SCREEN,
			GLOW_MODE_SOFTLIGHT,
			GLOW_MODE_REPLACE,
			GLOW_MODE_MIX
		};

		GlowMode glow_mode = GLOW_MODE_ADD;
		float glow_intensity = 1.0;
		uint32_t glow_level_flags = 0;
		Vector2i glow_texture_size;
		bool glow_use_bicubic_upscale = false;
		RID glow_texture;

		RS::EnvironmentToneMapper tonemap_mode = RS::ENV_TONE_MAPPER_LINEAR;
		float exposure = 1.0;
		float white = 1.0;

		bool use_auto_exposure = false;
		float auto_exposure_grey = 0.5;
		RID exposure_texture;

		bool use_bcs = false;
		float brightness = 1.0;
		float contrast = 1.0;
		float saturation = 1.0;

		bool use_color_correction = false;
		RID color_correction_texture;
	};

	void tonemapper(RID p_source_color, RID p_dst_framebuffer, const TonemapSettings &p_settings);

	void generate_ssao(RID p_depth_buffer, RID p_normal_buffer, const Size2i &p_depth_buffer_size, RID p_depth_mipmaps_texture, const Vector<RID> &depth_mipmaps, RID p_ao1, bool p_half_size, RID p_ao2, RID p_upscale_buffer, float p_intensity, float p_radius, float p_bias, const CameraMatrix &p_projection, RS::EnvironmentSSAOQuality p_quality, RS::EnvironmentSSAOBlur p_blur, float p_edge_sharpness);

	void roughness_limit(RID p_source_normal, RID p_roughness, const Size2i &p_size, float p_curve);
	void cubemap_downsample(RID p_source_cubemap, RID p_dest_cubemap, const Size2i &p_size);
	void cubemap_filter(RID p_source_cubemap, Vector<RID> p_dest_cubemap, bool p_use_array);
	void render_sky(RD::DrawListID p_list, float p_time, RID p_fb, RID p_samplers, RID p_lights, RenderPipelineVertexFormatCacheRD *p_pipeline, RID p_uniform_set, RID p_texture_set, const CameraMatrix &p_camera, const Basis &p_orientation, float p_multiplier, const Vector3 &p_position);

	void screen_space_reflection(RID p_diffuse, RID p_normal, RS::EnvironmentSSRRoughnessQuality p_roughness_quality, RID p_roughness, RID p_blur_radius, RID p_blur_radius2, RID p_metallic, const Color &p_metallic_mask, RID p_depth, RID p_scale_depth, RID p_scale_normal, RID p_output, RID p_output_blur, const Size2i &p_screen_size, int p_max_steps, float p_fade_in, float p_fade_out, float p_tolerance, const CameraMatrix &p_camera);
	void merge_specular(RID p_dest_framebuffer, RID p_specular, RID p_base, RID p_reflection);
	void sub_surface_scattering(RID p_diffuse, RID p_diffuse2, RID p_depth, const CameraMatrix &p_camera, const Size2i &p_screen_size, float p_scale, float p_depth_scale, RS::SubSurfaceScatteringQuality p_quality);

	RasterizerEffectsRD();
	~RasterizerEffectsRD();
};

#endif // !RASTERIZER_EFFECTS_RD_H
