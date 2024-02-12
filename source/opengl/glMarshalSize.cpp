#include "boxedwine.h"

#ifdef BOXEDWINE_OPENGL
#include GLH
#include "glcommon.h"
#include "glMarshal.h"

U32 getDataSize(GLenum type) {
    switch (type) {
    case GL_UNSIGNED_BYTE:
    case GL_BYTE: 
        return 1;
    case GL_SHORT:
    case GL_UNSIGNED_SHORT: 
        return 2;
    case GL_INT:
    case GL_UNSIGNED_INT:
    case GL_FLOAT:
        return 4;
    case GL_DOUBLE:
        return 8;

    case GL_V2F:
        return 8;
    case GL_V3F:
        return 12;
    case GL_C4UB_V2F:
        return 12;
    case GL_C4UB_V3F:
        return 16;
    case GL_C3F_V3F:
        return 24;
    case GL_N3F_V3F:
        return 24;
    case GL_C4F_N3F_V3F:
        return 40;
    case GL_T2F_V3F:
        return 20;
    case GL_T4F_V4F:
        return 32;
    case GL_T2F_C4UB_V3F:
        return 36;
    case GL_T2F_C3F_V3F:
        return 32;
    case GL_T2F_N3F_V3F:
        return 32;
    case GL_T2F_C4F_N3F_V3F:
        return 48;
    case GL_T4F_C4F_N3F_V4F:
        return 60;
    default:
        kpanic("glcommon.c getDataSize unknown type: %d", type);
        return 4;
    }
}


int getSize(GLenum pname) {
    switch (pname) {
      case GL_ACCUM_RED_BITS:
      case GL_ACCUM_GREEN_BITS:
      case GL_ACCUM_BLUE_BITS:
      case GL_ACCUM_ALPHA_BITS:
      case GL_ALPHA_BIAS:
      case GL_ALPHA_BITS:
      case GL_ALPHA_SCALE:
      case GL_ALPHA_TEST:
      case GL_ALPHA_TEST_FUNC:
      case GL_ALPHA_TEST_REF:
      case GL_ATTRIB_STACK_DEPTH:
      case GL_AUTO_NORMAL:
      case GL_AUX_BUFFERS:
      case GL_BLEND:
      case GL_BLEND_DST:
      case GL_BLEND_SRC:
      case GL_BLEND_SRC_RGB_EXT:
      case GL_BLEND_DST_RGB_EXT:
      case GL_BLEND_SRC_ALPHA_EXT:
      case GL_BLEND_DST_ALPHA_EXT:
      case GL_BLEND_EQUATION:
      case GL_BLEND_EQUATION_ALPHA_EXT:
      case GL_BLUE_BIAS:
      case GL_BLUE_BITS:
      case GL_BLUE_SCALE:
      case GL_CLIENT_ATTRIB_STACK_DEPTH:
      case GL_CLIP_PLANE0:
      case GL_CLIP_PLANE1:
      case GL_CLIP_PLANE2:
      case GL_CLIP_PLANE3:
      case GL_CLIP_PLANE4:
      case GL_CLIP_PLANE5:	
      case GL_COLOR_MATERIAL:
      case GL_COLOR_MATERIAL_FACE:
      case GL_COLOR_MATERIAL_PARAMETER:
      case GL_CULL_FACE:
      case GL_CULL_FACE_MODE:	
      case GL_CURRENT_RASTER_DISTANCE:
      case GL_CURRENT_RASTER_INDEX:
      case GL_CURRENT_INDEX:		 
      case GL_CURRENT_RASTER_POSITION_VALID:
      case GL_DEPTH_BIAS:
      case GL_DEPTH_BITS:
      case GL_DEPTH_CLEAR_VALUE:
      case GL_DEPTH_FUNC:
      case GL_DEPTH_SCALE:
      case GL_DEPTH_TEST:
      case GL_DEPTH_WRITEMASK:
      case GL_DITHER:
      case GL_DOUBLEBUFFER:
      case GL_DRAW_BUFFER:
      case GL_EDGE_FLAG:
      case GL_FEEDBACK_BUFFER_SIZE:
      case GL_FEEDBACK_BUFFER_TYPE:
      case GL_FOG:
      case GL_FOG_DENSITY:
      case GL_FOG_END:
      case GL_FOG_HINT:
      case GL_FOG_INDEX:
      case GL_FOG_MODE:
      case GL_FOG_START:
      case GL_FRONT_FACE:
      case GL_GREEN_BIAS:
      case GL_GREEN_BITS:
      case GL_GREEN_SCALE:
      case GL_INDEX_BITS:
      case GL_INDEX_CLEAR_VALUE:
      case GL_INDEX_MODE:
      case GL_INDEX_OFFSET:
      case GL_INDEX_SHIFT:
      case GL_INDEX_WRITEMASK:
      case GL_LIGHT0:
      case GL_LIGHT1:
      case GL_LIGHT2:
      case GL_LIGHT3:
      case GL_LIGHT4:
      case GL_LIGHT5:
      case GL_LIGHT6:
      case GL_LIGHT7:
      case GL_LIGHTING:
      case GL_LIGHT_MODEL_COLOR_CONTROL:
      case GL_LIGHT_MODEL_LOCAL_VIEWER:
      case GL_LIGHT_MODEL_TWO_SIDE:
      case GL_LINE_SMOOTH:
      case GL_LINE_SMOOTH_HINT:
      case GL_LINE_STIPPLE:
      case GL_LINE_STIPPLE_PATTERN:
      case GL_LINE_STIPPLE_REPEAT:
      case GL_LINE_WIDTH:
      case GL_LINE_WIDTH_GRANULARITY:		
      case GL_LIST_BASE:
      case GL_LIST_INDEX:
      case GL_LIST_MODE:
      case GL_INDEX_LOGIC_OP:
      case GL_COLOR_LOGIC_OP:
      case GL_LOGIC_OP_MODE:
      case GL_MAP1_COLOR_4:		      
      case GL_MAP1_GRID_SEGMENTS:
      case GL_MAP1_INDEX:
      case GL_MAP1_NORMAL:
      case GL_MAP1_TEXTURE_COORD_1:
      case GL_MAP1_TEXTURE_COORD_2:
      case GL_MAP1_TEXTURE_COORD_3:
      case GL_MAP1_TEXTURE_COORD_4:
      case GL_MAP1_VERTEX_3:
      case GL_MAP1_VERTEX_4:
      case GL_MAP2_COLOR_4:
      case GL_MAP2_INDEX:
      case GL_MAP2_NORMAL:
      case GL_MAP2_TEXTURE_COORD_1:
      case GL_MAP2_TEXTURE_COORD_2:
      case GL_MAP2_TEXTURE_COORD_3:
      case GL_MAP2_TEXTURE_COORD_4:
      case GL_MAP2_VERTEX_3:
      case GL_MAP2_VERTEX_4:
      case GL_MAP_COLOR:
      case GL_MAP_STENCIL:
      case GL_MATRIX_MODE:
      case GL_MAX_ATTRIB_STACK_DEPTH:
      case GL_MAX_CLIENT_ATTRIB_STACK_DEPTH:
      case GL_MAX_CLIP_PLANES:
      case GL_MAX_ELEMENTS_VERTICES:
      case GL_MAX_ELEMENTS_INDICES:
      case GL_MAX_EVAL_ORDER:
      case GL_MAX_LIGHTS:
      case GL_MAX_LIST_NESTING:
      case GL_MAX_MODELVIEW_STACK_DEPTH:
      case GL_MAX_NAME_STACK_DEPTH:
      case GL_MAX_PIXEL_MAP_TABLE:
      case GL_MAX_PROJECTION_STACK_DEPTH:
      case GL_MAX_TEXTURE_SIZE:
      case GL_MAX_3D_TEXTURE_SIZE:
      case GL_MAX_TEXTURE_STACK_DEPTH:		  		
      case GL_MODELVIEW_STACK_DEPTH:
      case GL_NAME_STACK_DEPTH:
      case GL_NORMALIZE:
      case GL_PACK_ALIGNMENT:
      case GL_PACK_LSB_FIRST:
      case GL_PACK_ROW_LENGTH:
      case GL_PACK_SKIP_PIXELS:
      case GL_PACK_SKIP_ROWS:
      case GL_PACK_SWAP_BYTES:
      case GL_PACK_SKIP_IMAGES_EXT:
      case GL_PACK_IMAGE_HEIGHT_EXT:
      case GL_PACK_INVERT_MESA:
      case GL_PERSPECTIVE_CORRECTION_HINT:
      case GL_PIXEL_MAP_A_TO_A_SIZE:
      case GL_PIXEL_MAP_B_TO_B_SIZE:
      case GL_PIXEL_MAP_G_TO_G_SIZE:
      case GL_PIXEL_MAP_I_TO_A_SIZE:
      case GL_PIXEL_MAP_I_TO_B_SIZE:
      case GL_PIXEL_MAP_I_TO_G_SIZE:
      case GL_PIXEL_MAP_I_TO_I_SIZE:
      case GL_PIXEL_MAP_I_TO_R_SIZE:
      case GL_PIXEL_MAP_R_TO_R_SIZE:
      case GL_PIXEL_MAP_S_TO_S_SIZE:
      case GL_POINT_SIZE:
      case GL_POINT_SIZE_GRANULARITY:
      case GL_POINT_SMOOTH:
      case GL_POINT_SMOOTH_HINT:
      case GL_POINT_SIZE_MIN_EXT:
      case GL_POINT_SIZE_MAX_EXT:
      case GL_POINT_FADE_THRESHOLD_SIZE_EXT:	  
      case GL_POLYGON_OFFSET_BIAS_EXT:
      case GL_POLYGON_OFFSET_FACTOR:
      case GL_POLYGON_OFFSET_UNITS:
      case GL_POLYGON_OFFSET_POINT:
      case GL_POLYGON_OFFSET_LINE:
      case GL_POLYGON_OFFSET_FILL:
      case GL_POLYGON_SMOOTH:
      case GL_POLYGON_SMOOTH_HINT:
      case GL_POLYGON_STIPPLE:		  
      case GL_PROJECTION_STACK_DEPTH:
      case GL_READ_BUFFER:
      case GL_RED_BIAS:
      case GL_RED_BITS:
      case GL_RED_SCALE:
      case GL_RENDER_MODE:
      case GL_RESCALE_NORMAL:
      case GL_RGBA_MODE:		 
      case GL_SCISSOR_TEST:
      case GL_SELECTION_BUFFER_SIZE:
      case GL_SHADE_MODEL:
      case GL_SHARED_TEXTURE_PALETTE_EXT:
      case GL_STENCIL_BITS:
      case GL_STENCIL_CLEAR_VALUE:
      case GL_STENCIL_FAIL:
      case GL_STENCIL_FUNC:
      case GL_STENCIL_PASS_DEPTH_FAIL:
      case GL_STENCIL_PASS_DEPTH_PASS:
      case GL_STENCIL_REF:
      case GL_STENCIL_TEST:
      case GL_STENCIL_VALUE_MASK:
      case GL_STENCIL_WRITEMASK:
      case GL_STEREO:
      case GL_SUBPIXEL_BITS:
      case GL_TEXTURE_1D:
      case GL_TEXTURE_2D:
      case GL_TEXTURE_3D:
#ifdef GL_TEXTURE_1D_ARRAY_EXT
      case GL_TEXTURE_1D_ARRAY_EXT:
#endif
#ifdef GL_TEXTURE_2D_ARRAY_EXT
      case GL_TEXTURE_2D_ARRAY_EXT:
#endif
      case GL_TEXTURE_BINDING_1D:
      case GL_TEXTURE_BINDING_2D:
      case GL_TEXTURE_BINDING_3D:
#ifdef GL_TEXTURE_BINDING_1D_ARRAY_EXT
      case GL_TEXTURE_BINDING_1D_ARRAY_EXT:
#endif
#ifdef GL_TEXTURE_BINDING_2D_ARRAY_EXT
      case GL_TEXTURE_BINDING_2D_ARRAY_EXT:
#endif
      case GL_TEXTURE_GEN_S:
      case GL_TEXTURE_GEN_T:
      case GL_TEXTURE_GEN_R:
      case GL_TEXTURE_GEN_Q:		   
      case GL_TEXTURE_STACK_DEPTH:
      case GL_UNPACK_ALIGNMENT:
      case GL_UNPACK_LSB_FIRST:
      case GL_UNPACK_ROW_LENGTH:
      case GL_UNPACK_SKIP_PIXELS:
      case GL_UNPACK_SKIP_ROWS:
      case GL_UNPACK_SWAP_BYTES:
      case GL_UNPACK_SKIP_IMAGES_EXT:
      case GL_UNPACK_IMAGE_HEIGHT_EXT:
      case GL_UNPACK_CLIENT_STORAGE_APPLE:		  
      case GL_ZOOM_X:
      case GL_ZOOM_Y:
      case GL_VERTEX_ARRAY:
      case GL_VERTEX_ARRAY_SIZE:
      case GL_VERTEX_ARRAY_TYPE:
      case GL_VERTEX_ARRAY_STRIDE:
      case GL_VERTEX_ARRAY_COUNT_EXT:
      case GL_NORMAL_ARRAY:
      case GL_NORMAL_ARRAY_TYPE:
      case GL_NORMAL_ARRAY_STRIDE:
      case GL_NORMAL_ARRAY_COUNT_EXT:
      case GL_COLOR_ARRAY:
      case GL_COLOR_ARRAY_SIZE:
      case GL_COLOR_ARRAY_TYPE:
      case GL_COLOR_ARRAY_STRIDE:
      case GL_COLOR_ARRAY_COUNT_EXT:
      case GL_INDEX_ARRAY:
      case GL_INDEX_ARRAY_TYPE:
      case GL_INDEX_ARRAY_STRIDE:
      case GL_INDEX_ARRAY_COUNT_EXT:
      case GL_TEXTURE_COORD_ARRAY:
      case GL_TEXTURE_COORD_ARRAY_SIZE:
      case GL_TEXTURE_COORD_ARRAY_TYPE:
      case GL_TEXTURE_COORD_ARRAY_STRIDE:
      case GL_TEXTURE_COORD_ARRAY_COUNT_EXT:
      case GL_EDGE_FLAG_ARRAY:
      case GL_EDGE_FLAG_ARRAY_STRIDE:
      case GL_EDGE_FLAG_ARRAY_COUNT_EXT:
      case GL_MAX_TEXTURE_UNITS_ARB:
      case GL_ACTIVE_TEXTURE_ARB:
      case GL_CLIENT_ACTIVE_TEXTURE_ARB:
      case GL_TEXTURE_CUBE_MAP_ARB:
      case GL_TEXTURE_BINDING_CUBE_MAP_ARB:
      case GL_MAX_CUBE_MAP_TEXTURE_SIZE_ARB:
      case GL_TEXTURE_COMPRESSION_HINT_ARB:
      case GL_NUM_COMPRESSED_TEXTURE_FORMATS_ARB:
      case GL_ARRAY_ELEMENT_LOCK_FIRST_EXT:
      case GL_ARRAY_ELEMENT_LOCK_COUNT_EXT:		  		 
      case GL_COLOR_MATRIX_STACK_DEPTH_SGI:
      case GL_MAX_COLOR_MATRIX_STACK_DEPTH_SGI:
      case GL_POST_COLOR_MATRIX_RED_SCALE_SGI:
      case GL_POST_COLOR_MATRIX_GREEN_SCALE_SGI:
      case GL_POST_COLOR_MATRIX_BLUE_SCALE_SGI:
      case GL_POST_COLOR_MATRIX_ALPHA_SCALE_SGI:
      case GL_POST_COLOR_MATRIX_RED_BIAS_SGI:
      case GL_POST_COLOR_MATRIX_GREEN_BIAS_SGI:
      case GL_POST_COLOR_MATRIX_BLUE_BIAS_SGI:
      case GL_POST_COLOR_MATRIX_ALPHA_BIAS_SGI:
      case GL_CONVOLUTION_1D_EXT:
      case GL_CONVOLUTION_2D_EXT:
      case GL_SEPARABLE_2D_EXT:
      case GL_POST_CONVOLUTION_RED_SCALE_EXT:
      case GL_POST_CONVOLUTION_GREEN_SCALE_EXT:
      case GL_POST_CONVOLUTION_BLUE_SCALE_EXT:
      case GL_POST_CONVOLUTION_ALPHA_SCALE_EXT:
      case GL_POST_CONVOLUTION_RED_BIAS_EXT:
      case GL_POST_CONVOLUTION_GREEN_BIAS_EXT:
      case GL_POST_CONVOLUTION_BLUE_BIAS_EXT:
      case GL_POST_CONVOLUTION_ALPHA_BIAS_EXT:
      case GL_HISTOGRAM:
      case GL_MINMAX:
      case GL_COLOR_TABLE_SGI:
      case GL_POST_CONVOLUTION_COLOR_TABLE_SGI:
      case GL_POST_COLOR_MATRIX_COLOR_TABLE_SGI:
      case GL_TEXTURE_COLOR_TABLE_SGI:
      case GL_COLOR_SUM_EXT:		   
      case GL_SECONDARY_COLOR_ARRAY_EXT:
      case GL_SECONDARY_COLOR_ARRAY_TYPE_EXT:
      case GL_SECONDARY_COLOR_ARRAY_STRIDE_EXT:
      case GL_SECONDARY_COLOR_ARRAY_SIZE_EXT:
      case GL_CURRENT_FOG_COORDINATE_EXT:
      case GL_FOG_COORDINATE_ARRAY_EXT:
      case GL_FOG_COORDINATE_ARRAY_TYPE_EXT:
      case GL_FOG_COORDINATE_ARRAY_STRIDE_EXT:
      case GL_FOG_COORDINATE_SOURCE_EXT:
      case GL_MAX_TEXTURE_LOD_BIAS_EXT:
      case GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT:
      case GL_MULTISAMPLE_ARB:
      case GL_SAMPLE_ALPHA_TO_COVERAGE_ARB:
      case GL_SAMPLE_ALPHA_TO_ONE_ARB:
      case GL_SAMPLE_COVERAGE_ARB:
      case GL_SAMPLE_COVERAGE_VALUE_ARB:
      case GL_SAMPLE_COVERAGE_INVERT_ARB:
      case GL_SAMPLE_BUFFERS_ARB:
      case GL_SAMPLES_ARB:
      case GL_RASTER_POSITION_UNCLIPPED_IBM:
      case GL_POINT_SPRITE_NV:
      case GL_POINT_SPRITE_R_MODE_NV:
      case GL_POINT_SPRITE_COORD_ORIGIN:
      case GL_GENERATE_MIPMAP_HINT_SGIS:
      case GL_VERTEX_PROGRAM_BINDING_NV:
      case GL_VERTEX_ATTRIB_ARRAY0_NV:
      case GL_VERTEX_ATTRIB_ARRAY1_NV:
      case GL_VERTEX_ATTRIB_ARRAY2_NV:
      case GL_VERTEX_ATTRIB_ARRAY3_NV:
      case GL_VERTEX_ATTRIB_ARRAY4_NV:
      case GL_VERTEX_ATTRIB_ARRAY5_NV:
      case GL_VERTEX_ATTRIB_ARRAY6_NV:
      case GL_VERTEX_ATTRIB_ARRAY7_NV:
      case GL_VERTEX_ATTRIB_ARRAY8_NV:
      case GL_VERTEX_ATTRIB_ARRAY9_NV:
      case GL_VERTEX_ATTRIB_ARRAY10_NV:
      case GL_VERTEX_ATTRIB_ARRAY11_NV:
      case GL_VERTEX_ATTRIB_ARRAY12_NV:
      case GL_VERTEX_ATTRIB_ARRAY13_NV:
      case GL_VERTEX_ATTRIB_ARRAY14_NV:
      case GL_VERTEX_ATTRIB_ARRAY15_NV:
      case GL_MAP1_VERTEX_ATTRIB0_4_NV:
      case GL_MAP1_VERTEX_ATTRIB1_4_NV:
      case GL_MAP1_VERTEX_ATTRIB2_4_NV:
      case GL_MAP1_VERTEX_ATTRIB3_4_NV:
      case GL_MAP1_VERTEX_ATTRIB4_4_NV:
      case GL_MAP1_VERTEX_ATTRIB5_4_NV:
      case GL_MAP1_VERTEX_ATTRIB6_4_NV:
      case GL_MAP1_VERTEX_ATTRIB7_4_NV:
      case GL_MAP1_VERTEX_ATTRIB8_4_NV:
      case GL_MAP1_VERTEX_ATTRIB9_4_NV:
      case GL_MAP1_VERTEX_ATTRIB10_4_NV:
      case GL_MAP1_VERTEX_ATTRIB11_4_NV:
      case GL_MAP1_VERTEX_ATTRIB12_4_NV:
      case GL_MAP1_VERTEX_ATTRIB13_4_NV:
      case GL_MAP1_VERTEX_ATTRIB14_4_NV:
      case GL_MAP1_VERTEX_ATTRIB15_4_NV:
      case GL_FRAGMENT_PROGRAM_NV:
      case GL_FRAGMENT_PROGRAM_BINDING_NV:
      case GL_MAX_FRAGMENT_PROGRAM_LOCAL_PARAMETERS_NV:
      case GL_TEXTURE_RECTANGLE_NV:
      case GL_TEXTURE_BINDING_RECTANGLE_NV:
      case GL_MAX_RECTANGLE_TEXTURE_SIZE_NV:
      case GL_STENCIL_TEST_TWO_SIDE_EXT:
      case GL_ACTIVE_STENCIL_FACE_EXT:
      case GL_MAX_SHININESS_NV:
      case GL_MAX_SPOT_EXPONENT_NV:
      case GL_ARRAY_BUFFER_BINDING_ARB:
      case GL_VERTEX_ARRAY_BUFFER_BINDING_ARB:
      case GL_NORMAL_ARRAY_BUFFER_BINDING_ARB:
      case GL_COLOR_ARRAY_BUFFER_BINDING_ARB:
      case GL_INDEX_ARRAY_BUFFER_BINDING_ARB:
      case GL_TEXTURE_COORD_ARRAY_BUFFER_BINDING_ARB:
      case GL_EDGE_FLAG_ARRAY_BUFFER_BINDING_ARB:
      case GL_SECONDARY_COLOR_ARRAY_BUFFER_BINDING_ARB:
      case GL_FOG_COORDINATE_ARRAY_BUFFER_BINDING_ARB:
      case GL_ELEMENT_ARRAY_BUFFER_BINDING_ARB:
      case GL_PIXEL_PACK_BUFFER_BINDING_EXT:
      case GL_PIXEL_UNPACK_BUFFER_BINDING_EXT:
      case GL_VERTEX_PROGRAM_ARB:
      case GL_VERTEX_PROGRAM_POINT_SIZE_ARB:
      case GL_VERTEX_PROGRAM_TWO_SIDE_ARB:
      case GL_MAX_PROGRAM_MATRIX_STACK_DEPTH_ARB:
      case GL_MAX_PROGRAM_MATRICES_ARB:
      case GL_CURRENT_MATRIX_STACK_DEPTH_ARB:		  
      case GL_MAX_VERTEX_ATTRIBS_ARB:
      case GL_PROGRAM_ERROR_POSITION_ARB:
      case GL_FRAGMENT_PROGRAM_ARB:
      case GL_MAX_TEXTURE_COORDS_ARB:
      case GL_MAX_TEXTURE_IMAGE_UNITS_ARB:
      case GL_DEPTH_BOUNDS_TEST_EXT:
#ifdef GL_DEPTH_CLAMP
      case GL_DEPTH_CLAMP:
#endif
      case GL_MAX_DRAW_BUFFERS_ARB:
      case GL_DRAW_BUFFER0_ARB:
      case GL_DRAW_BUFFER1_ARB:
      case GL_DRAW_BUFFER2_ARB:
      case GL_DRAW_BUFFER3_ARB:
      case GL_IMPLEMENTATION_COLOR_READ_TYPE_OES:
      case GL_IMPLEMENTATION_COLOR_READ_FORMAT_OES:
      case GL_NUM_FRAGMENT_REGISTERS_ATI:
      case GL_NUM_FRAGMENT_CONSTANTS_ATI:
      case GL_NUM_PASSES_ATI:
      case GL_NUM_INSTRUCTIONS_PER_PASS_ATI:
      case GL_NUM_INSTRUCTIONS_TOTAL_ATI:
      case GL_COLOR_ALPHA_PAIRING_ATI:
      case GL_NUM_LOOPBACK_COMPONENTS_ATI:
      case GL_NUM_INPUT_INTERPOLATOR_COMPONENTS_ATI:
      case GL_STENCIL_BACK_FUNC:
      case GL_STENCIL_BACK_VALUE_MASK:
      case GL_STENCIL_BACK_WRITEMASK:
      case GL_STENCIL_BACK_REF:
      case GL_STENCIL_BACK_FAIL:
      case GL_STENCIL_BACK_PASS_DEPTH_FAIL:
      case GL_STENCIL_BACK_PASS_DEPTH_PASS:
      case GL_FRAMEBUFFER_BINDING_EXT:
      case GL_RENDERBUFFER_BINDING_EXT:
      case GL_MAX_COLOR_ATTACHMENTS_EXT:
      case GL_MAX_RENDERBUFFER_SIZE_EXT:
#ifdef GL_READ_FRAMEBUFFER_BINDING_EXT
      case GL_READ_FRAMEBUFFER_BINDING_EXT:
#endif
#ifdef GL_PROVOKING_VERTEX_EXT
      case GL_PROVOKING_VERTEX_EXT:
#endif
#ifdef GL_QUADS_FOLLOW_PROVOKING_VERTEX_CONVENTION_EXT
      case GL_QUADS_FOLLOW_PROVOKING_VERTEX_CONVENTION_EXT:
#endif
      case GL_MAX_FRAGMENT_UNIFORM_COMPONENTS_ARB:
      case GL_FRAGMENT_SHADER_DERIVATIVE_HINT_ARB:
      case GL_MAX_VERTEX_UNIFORM_COMPONENTS_ARB:
      case GL_MAX_VARYING_FLOATS_ARB:
      case GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS_ARB:
      case GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS_ARB:
      case GL_CURRENT_PROGRAM:
#ifdef GL_MAX_SAMPLES
      case GL_MAX_SAMPLES:
#endif
      case GL_VERTEX_ARRAY_BINDING_APPLE:
#ifdef GL_TEXTURE_CUBE_MAP_SEAMLESS
      case GL_TEXTURE_CUBE_MAP_SEAMLESS:
#endif
#ifdef GL_MAX_SERVER_WAIT_TIMEOUT
      case GL_MAX_SERVER_WAIT_TIMEOUT:
#endif
#ifdef GL_NUM_EXTENSIONS
      case GL_NUM_EXTENSIONS:
#endif
#ifdef GL_MAJOR_VERSION
      case GL_MAJOR_VERSION:
#endif
#ifdef GL_MINOR_VERSION
      case GL_MINOR_VERSION:
#endif
      case 0x854d: // GL_MAX_GENERAL_COMBINERS_NV
      case 0x9126: // GL_CONTEXT_PROFILE_MASK
      case 0x90bc: // GL_MIN_MAP_BUFFER_ALIGNMENT
      case 0x8a28: // GL_UNIFORM_BUFFER_BINDING
      case 0x8a29: // GL_UNIFORM_BUFFER_START
      case 0x8a2a: // GL_UNIFORM_BUFFER_SIZE
      case 0x8a2b: // GL_MAX_VERTEX_UNIFORM_BLOCKS
      case 0x8a2c: // GL_MAX_GEOMETRY_UNIFORM_BLOCKS
      case 0x8a2d: // GL_MAX_FRAGMENT_UNIFORM_BLOCKS
      case 0x8a2e: // GL_MAX_COMBINED_UNIFORM_BLOCKS
      case 0x8a2f: // GL_MAX_UNIFORM_BUFFER_BINDINGS
      case 0x9125: // GL_MAX_FRAGMENT_INPUT_COMPONENTS
      case 0x8E71: // GL_MAX_VERTEX_STREAMS
      case 0x8C29: // GL_MAX_GEOMETRY_TEXTURE_IMAGE_UNITS_EXT
      case 0x8e82: // GL_MAX_TESS_EVALUATION_TEXTURE_IMAGE_UNITS
      case 0x8e8a: // GL_MAX_TESS_EVALUATION_UNIFORM_BLOCKS
      case 0x8e81: // GL_MAX_TESS_CONTROL_TEXTURE_IMAGE_UNITS
      case 0x8e89: // GL_MAX_TESS_CONTROL_UNIFORM_BLOCKS
      case 0x825c: // GL_VIEWPORT_SUBPIXEL_BITS
      case GL_MAX_FRAMEBUFFER_HEIGHT:
      case GL_MAX_FRAMEBUFFER_WIDTH:
      case GL_MAX_ATOMIC_COUNTER_BUFFER_BINDINGS:
      case GL_MAX_COMPUTE_UNIFORM_BLOCKS:
      case GL_MAX_COMPUTE_TEXTURE_IMAGE_UNITS:
      case GL_TEXTURE_BUFFER_OFFSET_ALIGNMENT:
      case GL_MAX_FRAGMENT_ATOMIC_COUNTER_BUFFERS:
      case GL_MAX_COMBINED_ATOMIC_COUNTER_BUFFERS:
        return 1;
      case GL_DEPTH_BOUNDS_EXT:
      case GL_DEPTH_RANGE:
      case GL_LINE_WIDTH_RANGE:
      case GL_ALIASED_LINE_WIDTH_RANGE:
      case GL_MAP1_GRID_DOMAIN:
      case GL_MAP2_GRID_SEGMENTS:
      case GL_MAX_VIEWPORT_DIMS:
      case GL_POINT_SIZE_RANGE:
      case GL_ALIASED_POINT_SIZE_RANGE:
      case GL_POLYGON_MODE:
         return 2;
        case GL_CURRENT_NORMAL:
        case GL_DISTANCE_ATTENUATION_EXT:
            return 3;
        case GL_ACCUM_CLEAR_VALUE:
        case GL_BLEND_COLOR_EXT:
        case GL_COLOR_CLEAR_VALUE:
        case GL_COLOR_WRITEMASK:
        case GL_CURRENT_COLOR:
        case GL_CURRENT_RASTER_COLOR:
        case GL_CURRENT_RASTER_POSITION:
#ifdef GL_CURRENT_RASTER_SECONDARY_COLOR
        case GL_CURRENT_RASTER_SECONDARY_COLOR:
#endif
        case GL_CURRENT_RASTER_TEXTURE_COORDS:
        case GL_CURRENT_TEXTURE_COORDS:
        case GL_FOG_COLOR:
        case GL_LIGHT_MODEL_AMBIENT:
        case GL_MAP2_GRID_DOMAIN:
        case GL_SCISSOR_BOX:
        case GL_VIEWPORT:
        case GL_CURRENT_SECONDARY_COLOR_EXT:
         return 4;
      case GL_MODELVIEW_MATRIX:
      case GL_PROJECTION_MATRIX:
      case GL_TEXTURE_MATRIX:
      case GL_TRANSPOSE_COLOR_MATRIX_ARB:
      case GL_TRANSPOSE_MODELVIEW_MATRIX_ARB:
      case GL_TRANSPOSE_PROJECTION_MATRIX_ARB:
      case GL_TRANSPOSE_TEXTURE_MATRIX_ARB:
      case GL_COLOR_MATRIX_SGI:
      case GL_CURRENT_MATRIX_ARB:
      case GL_TRANSPOSE_CURRENT_MATRIX_ARB:
         return 16;
      
      case GL_COMPRESSED_TEXTURE_FORMATS_ARB: {
          GLint results;
          glGetIntegerv(GL_NUM_COMPRESSED_TEXTURE_FORMATS_ARB, &results);
          return results;
      }      
      default:
          klog("Unknown pname for get: %d", pname);
          return 1;
   }
}

int glcommon_glLightv_size(GLenum e)
{
    switch (e) {
    case GL_SPOT_EXPONENT:
    case GL_SPOT_CUTOFF:
    case GL_CONSTANT_ATTENUATION:
    case GL_LINEAR_ATTENUATION:
    case GL_QUADRATIC_ATTENUATION:
        return 1;
    case GL_SPOT_DIRECTION:
        return 3;
    case GL_AMBIENT:
    case GL_DIFFUSE:
    case GL_SPECULAR:
    case GL_POSITION:
        return 4;
    default:
        return 0;
    }
}

int glcommon_glLightModelv_size(GLenum e)
{
    switch (e) {
    case GL_LIGHT_MODEL_LOCAL_VIEWER:
    case GL_LIGHT_MODEL_TWO_SIDE:
    case GL_LIGHT_MODEL_COLOR_CONTROL:
        return 1;
    case GL_LIGHT_MODEL_AMBIENT:
        return 4;
    default:
        return 0;
    }
}

int glcommon_glMaterialv_size(GLenum e)
{
    switch (e) {
    case GL_SHININESS:
        return 1;
    case GL_COLOR_INDEXES:
        return 3;
    case GL_AMBIENT:
    case GL_DIFFUSE:
    case GL_SPECULAR:
    case GL_EMISSION:
    case GL_AMBIENT_AND_DIFFUSE:
        return 4;
    default:
        return 0;
    }
}

// from mesa
GLint components_in_format(GLenum format )
{
   switch (format) {
      case GL_COLOR_INDEX:
      case GL_COLOR_INDEX1_EXT:
      case GL_COLOR_INDEX2_EXT:
      case GL_COLOR_INDEX4_EXT:
      case GL_COLOR_INDEX8_EXT:
      case GL_COLOR_INDEX12_EXT:
      case GL_COLOR_INDEX16_EXT:
      case GL_STENCIL_INDEX:
      case GL_DEPTH_COMPONENT:
      case GL_RED:
      case GL_GREEN:
      case GL_BLUE:
      case GL_ALPHA:
      case GL_LUMINANCE:
      case GL_INTENSITY:
         return 1;
      case GL_LUMINANCE_ALPHA:
      case GL_RG:
     return 2;
      case GL_RGB:
     return 3;
      case GL_RGBA:
     return 4;
      case GL_BGR:
     return 3;
      case GL_BGRA:
     return 4;
      case GL_ABGR_EXT:
         return 4;
      case GL_YCBCR_MESA:
         return 2;
#ifdef GL_DEPTH_STENCIL_EXT
      case GL_DEPTH_STENCIL_EXT:
         return 2;
#endif
      case GL_DUDV_ATI:
      case GL_DU8DV8_ATI:
         return 2;
      default:
          kpanic("Opengl components_in_format %d", format);
         return -1;
   }
}

// from mesa
GLint get_bytes_per_pixel(GLenum format, GLenum type)
{
   GLint comps = components_in_format (format );
   if (comps < 0)
      return -1;

   switch (type) {
      case GL_BITMAP:
         return 0;  /* special case */
      case GL_BYTE:
      case GL_UNSIGNED_BYTE:
         return comps * sizeof(GLubyte);
      case GL_SHORT:
      case GL_UNSIGNED_SHORT:
         return comps * sizeof(GLshort);
      case GL_INT:
      case GL_UNSIGNED_INT:
         return comps * sizeof(GLint);
      case GL_FLOAT:
         return comps * sizeof(GLfloat);
      case GL_HALF_FLOAT_ARB:
         return comps * sizeof(GLhalfARB);
      case GL_UNSIGNED_BYTE_3_3_2:
      case GL_UNSIGNED_BYTE_2_3_3_REV:
         if (format == GL_RGB || format == GL_BGR)
            return sizeof(GLubyte);
         else
            return -1;  /* error */
      case GL_UNSIGNED_SHORT_5_6_5:
      case GL_UNSIGNED_SHORT_5_6_5_REV:
         if (format == GL_RGB || format == GL_BGR)
            return sizeof(GLushort);
         else
            return -1;  /* error */
      case GL_UNSIGNED_SHORT_4_4_4_4:
      case GL_UNSIGNED_SHORT_4_4_4_4_REV:
      case GL_UNSIGNED_SHORT_5_5_5_1:
      case GL_UNSIGNED_SHORT_1_5_5_5_REV:
         if (format == GL_RGBA || format == GL_BGRA || format == GL_ABGR_EXT)
            return sizeof(GLushort);
         else
            return -1;
      case GL_UNSIGNED_INT_8_8_8_8:
      case GL_UNSIGNED_INT_8_8_8_8_REV:
      case GL_UNSIGNED_INT_10_10_10_2:
      case GL_UNSIGNED_INT_2_10_10_10_REV:
         if (format == GL_RGBA || format == GL_BGRA || format == GL_ABGR_EXT)
            return sizeof(GLuint);
         else
            return -1;
      case GL_UNSIGNED_SHORT_8_8_MESA:
      case GL_UNSIGNED_SHORT_8_8_REV_MESA:
         if (format == GL_YCBCR_MESA)
            return sizeof(GLushort);
         else
            return -1;
#ifdef GL_UNSIGNED_INT_24_8_EXT
      case GL_UNSIGNED_INT_24_8_EXT:
         if (format == GL_DEPTH_STENCIL_EXT)
            return sizeof(GLuint);
         else
            return -1;
#endif
      default:
         return -1;
   }
}

U32 getMap1Count(GLenum target) {
    switch (target) {
    case GL_MAP1_INDEX:
    case GL_MAP1_TEXTURE_COORD_1:
        return 1;
    case GL_MAP1_TEXTURE_COORD_2:
        return 2;
    case GL_MAP1_VERTEX_3: 
    case GL_MAP1_NORMAL:
    case GL_MAP1_TEXTURE_COORD_3:
        return 3;
    case GL_MAP1_VERTEX_4:
    case GL_MAP1_COLOR_4:
    case GL_MAP1_TEXTURE_COORD_4:
        return 4;
    default:
        kpanic("unknown target in getMap1Count: %d", target);
        return 0;
    }
}

U32 getMap2Count(GLenum target) {
    switch (target) {
    case GL_MAP2_INDEX:
    case GL_MAP2_TEXTURE_COORD_1:
        return 1;
    case GL_MAP2_TEXTURE_COORD_2:
        return 2;
    case GL_MAP2_VERTEX_3: 
    case GL_MAP2_NORMAL:
    case GL_MAP2_TEXTURE_COORD_3:
        return 3;
    case GL_MAP2_VERTEX_4:
    case GL_MAP2_COLOR_4:
    case GL_MAP2_TEXTURE_COORD_4:
        return 4;
    default:
        kpanic("unknown target in getMap2Count: %d", target);
        return 0;
    }
}

GLint glcommon_glGetPixelMap_size(GLenum map) {
    GLint len = 0;

    GL_FUNC(pglGetIntegerv)(map, &len);
    return len;
}

#ifndef DISABLE_GL_EXTENSIONS
#ifndef GL_ATOMIC_COUNTER_BUFFER_ACTIVE_ATOMIC_COUNTERS
#define GL_ATOMIC_COUNTER_BUFFER_ACTIVE_ATOMIC_COUNTERS 0x92C5
#endif

U32 marshalGetActiveAtomicCountersCount(U32 program, U32 bufferIndex) {
    GLint i=0;
    
    if (ext_glGetActiveAtomicCounterBufferiv)
        ext_glGetActiveAtomicCounterBufferiv(program, bufferIndex, GL_ATOMIC_COUNTER_BUFFER_ACTIVE_ATOMIC_COUNTERS, &i);
    return i;
}

#ifndef GL_NUM_COMPATIBLE_SUBROUTINES
#define GL_NUM_COMPATIBLE_SUBROUTINES     0x8E4A
#endif

U32 marshalGetCompatibleSubroutinesCount(U32 program, U32 shadertype, U32 index) {
    GLint i=0;
    if (ext_glGetActiveSubroutineUniformiv)
        ext_glGetActiveSubroutineUniformiv(program, shadertype, index, GL_NUM_COMPATIBLE_SUBROUTINES, &i);
    return i;
}

#ifndef GL_UNIFORM_BLOCK_ACTIVE_UNIFORMS
#define GL_UNIFORM_BLOCK_ACTIVE_UNIFORMS  0x8A42
#endif

U32 marshalGetUniformBlockActiveUnformsCount(U32 program, U32 uniformBlockIndex) {
    GLint i=0;
    if (ext_glGetActiveUniformBlockiv)
        ext_glGetActiveUniformBlockiv(program, uniformBlockIndex, GL_UNIFORM_BLOCK_ACTIVE_UNIFORMS, &i);
    return i;
}

U32 marshalGetColorTableWidth(U32 target) {
    GLint i=0;
    if (ext_glGetColorTableParameteriv)
        ext_glGetColorTableParameteriv(target, GL_COLOR_TABLE_WIDTH, &i);
    return i;
}

#ifndef GL_COLOR_TABLE_WIDTH_EXT
#define GL_COLOR_TABLE_WIDTH_EXT          0x80D9
#endif

U32 marshalGetColorTableWidthEXT(U32 target) {
    GLint i=0;
    if (ext_glGetColorTableParameterivEXT)
        ext_glGetColorTableParameterivEXT(target, GL_COLOR_TABLE_WIDTH_EXT, &i);
    return i;
}

U32 marshalGetColorTableWidthSGI(U32 target) {
    GLint i=0;
    if (ext_glGetColorTableParameterivSGI)
        ext_glGetColorTableParameterivSGI(target, GL_COLOR_TABLE_WIDTH_SGI, &i);
    return i;
}

U32 marshalGetCompressedImageSize(GLenum target, GLint level) {
    GLint i=0;
    if (ext_glGetTextureLevelParameteriv)
        ext_glGetTextureLevelParameteriv(target, level, GL_TEXTURE_COMPRESSED_IMAGE_SIZE, &i);
    return i;
}

U32 marshalGetCompressedImageSizeARB(GLenum target, GLint level) {
    GLint i=0;
    if (ext_glGetTextureLevelParameteriv)
        ext_glGetTextureLevelParameteriv(target, level, GL_TEXTURE_COMPRESSED_IMAGE_SIZE_ARB, &i);
    return i;
}

U32 marshalGetCompressedMultiImageSizeEXT(GLenum texunit, GLenum target, GLint level) {
    GLint i=0;
    if (ext_glGetMultiTexLevelParameterivEXT)
        ext_glGetMultiTexLevelParameterivEXT(texunit, target, level, GL_TEXTURE_COMPRESSED_IMAGE_SIZE, &i);
    return i;
}

U32 marshalGetCompressedTextureSizeEXT(GLuint texture, GLenum target, GLint lod) {
    GLint i=0;
    if (ext_glGetTextureLevelParameterivEXT)
        ext_glGetTextureLevelParameterivEXT(texture, target, lod, GL_TEXTURE_COMPRESSED_IMAGE_SIZE, &i);
    return i;
}

U32 marshalGetConvolutionWidth(U32 target) {
    GLint i = 0;

    if (ext_glGetConvolutionParameteriv) {
        ext_glGetConvolutionParameteriv(target, GL_CONVOLUTION_WIDTH, &i);
    }
    return i;
}

U32 marshalGetConvolutionHeight(U32 target) {
    GLint i = 0;

    if (ext_glGetConvolutionParameteriv) {
        ext_glGetConvolutionParameteriv(target, GL_CONVOLUTION_HEIGHT, &i);
    }
    return i;
}

GLsizei floatPerTransformList(GLenum transformType)
{
    switch (transformType) {
    case GL_NONE:
        return 0;
    case GL_TRANSLATE_X_NV:
    case GL_TRANSLATE_Y_NV:
        return 1;
    case GL_TRANSLATE_2D_NV:
        return 2;
    case GL_TRANSLATE_3D_NV:
        return 3;
    case GL_AFFINE_2D_NV:
    case GL_TRANSPOSE_AFFINE_2D_NV:
        return 6;
    case 0x9093: // GL_PROJECTIVE_2D_NV:
    case 0x9097: // GL_TRANSPOSE_PROJECTIVE_2D_NV:
        return 9;
    case GL_AFFINE_3D_NV:
    case GL_TRANSPOSE_AFFINE_3D_NV:
        return 12;
    case 0x9095: // GL_PROJECTIVE_3D_NV:
    case 0x9099: // GL_TRANSPOSE_PROJECTIVE_3D_NV:
        return 16;
    default:
        return 0;
    }
}

GLsizei marshalHistogramWidth(GLenum target) {
    GLint result = 0;
    if (ext_glGetHistogramParameteriv) {
        ext_glGetHistogramParameteriv(target, GL_HISTOGRAM_WIDTH, &result);
    }
    return result;
}

#endif
#endif
