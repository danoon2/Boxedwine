package boxedwine.org.parser;

import boxedwine.org.data.*;

import java.util.Arrays;
import java.util.HashSet;
import java.util.Set;
import java.util.Vector;

public class VkParserRules {
    static void run(VkData data) throws Exception {
        addDefaultTypes(data);
        updateStdVideoFields(data);
        removeUnsupportedFeatures(data);
        removeUnsupportedExtensions(data);

        // in vulkansc, maybe in future I should parse Features in xml to remove these
        removeFunction(data, "vkGetFaultData");
        removeFunction(data, "vkGetCommandPoolMemoryConsumption");

        data.stubMarshalWrite.add("VkDebugReportCallbackCreateInfoEXT"); // I don't think this is possible
        data.stubMarshalWrite.add("VkDebugUtilsMessengerCreateInfoEXT"); // I don't think this is possible
        data.stubMarshalWrite.add("VkAccelerationStructureBuildGeometryInfoKHR"); // I don't think this is possible
        data.stubMarshalWrite.add("VkIndirectCommandsLayoutTokenEXT"); // I don't think this is possible
        data.stubMarshalWrite.add("VkDescriptorGetInfoEXT"); // I don't think this is possible
        data.stubMarshalWrite.add("VkImportMemoryHostPointerInfoEXT"); // used by allocators which I don't support
        data.stubMarshalWrite.add("VkMemoryToImageCopy"); // :TODO: need to implement
        data.stubMarshalWrite.add("VkImageToMemoryCopy"); // :TODO: need to implement
        data.stubMarshalWrite.add("VkCuLaunchInfoNVX"); // :TODO: need to implement
        data.stubMarshalWrite.add("VkOpaqueCaptureDescriptorDataCreateInfoEXT"); // :TODO: need to implement
        data.stubMarshalWrite.add("VkOpticalFlowSessionCreatePrivateDataInfoNV"); // :TODO: need to implement
        data.stubMarshalWrite.add("VkMemoryMapPlacedInfoEXT"); // :TODO: need to implement
        data.stubMarshalWrite.add("VkRenderingInputAttachmentIndexInfo"); // :TODO: need to implement
        data.stubMarshalWrite.add("VkPushDescriptorSetWithTemplateInfo"); // :TODO: need to implement
        data.stubMarshalWrite.add("VkBindMemoryStatus"); // :TODO: need to implement
        data.stubMarshalWrite.add("VkQueryLowLatencySupportNV"); // :TODO: need to implement
        data.stubMarshalWrite.add("VkDeviceFaultInfoEXT"); // :TODO: need to implement
        data.stubMarshalWrite.add("VkCudaLaunchInfoNV"); // :TODO: need to implement
        data.stubMarshalWrite.add("VkMicromapBuildInfoEXT"); // :TODO: need to implement
        data.stubMarshalWrite.add("VkAccelerationStructureTrianglesOpacityMicromapEXT"); // :TODO: need to implement
        data.stubMarshalWrite.add("VkAccelerationStructureGeometryTrianglesDataKHR"); // :TODO: how to know if it is deviceAddress or hostAddress
        data.stubMarshalWrite.add("VkAccelerationStructureGeometryAabbsDataKHR"); // :TODO: how to know if it is deviceAddress or hostAddress
        data.stubMarshalWrite.add("VkAccelerationStructureGeometryInstancesDataKHR"); // :TODO: how to know if it is deviceAddress or hostAddress
        data.stubMarshalWrite.add("VkAccelerationStructureGeometryMotionTrianglesDataNV"); // :TODO: how to know if it is deviceAddress or hostAddress
        data.stubMarshalWrite.add("VkCopyAccelerationStructureToMemoryInfoKHR"); // :TODO: how to know if it is deviceAddress or hostAddress
        data.stubMarshalWrite.add("VkCopyMemoryToAccelerationStructureInfoKHR"); // :TODO: how to know if it is deviceAddress or hostAddress
        data.stubMarshalWrite.add("VkCopyMicromapToMemoryInfoEXT"); // :TODO: how to know if it is deviceAddress or hostAddress
        data.stubMarshalWrite.add("VkCopyMemoryToMicromapInfoEXT"); // :TODO: how to know if it is deviceAddress or hostAddress

        data.stubMarshalRead.add("VkMemoryToImageCopy"); // :TODO: need to implement
        data.stubMarshalRead.add("VkImageToMemoryCopy"); // :TODO: need to implement
        data.stubMarshalRead.add("StdVideoH264SequenceParameterSet"); // :TODO: need to implement
        data.stubMarshalRead.add("VkCuLaunchInfoNVX"); // :TODO: need to implement
        data.stubMarshalRead.add("VkOpaqueCaptureDescriptorDataCreateInfoEXT"); // :TODO: need to implement
        data.stubMarshalRead.add("VkOpticalFlowSessionCreatePrivateDataInfoNV"); // :TODO: need to implement
        data.stubMarshalRead.add("StdVideoAV1TileInfo"); // :TODO: need to implement
        data.stubMarshalRead.add("VkMemoryMapPlacedInfoEXT"); // :TODO: need to implement
        data.stubMarshalRead.add("VkRenderingInputAttachmentIndexInfo"); // :TODO: need to implement
        data.stubMarshalRead.add("VkPushDescriptorSetWithTemplateInfo"); // :TODO: need to implement
        data.stubMarshalRead.add("VkBindMemoryStatus"); // :TODO: need to implement
        data.stubMarshalRead.add("VkQueryLowLatencySupportNV"); // :TODO: need to implement
        data.stubMarshalRead.add("VkDeviceFaultInfoEXT"); // :TODO: need to implement
        data.stubMarshalRead.add("VkCudaLaunchInfoNV"); // :TODO: need to implement
        data.stubMarshalRead.add("VkMicromapBuildInfoEXT"); // :TODO: need to implement
        data.stubMarshalRead.add("StdVideoH265ShortTermRefPicSet"); // :TODO: need to implement
        data.stubMarshalRead.add("VkAccelerationStructureTrianglesOpacityMicromapEXT"); // :TODO: need to implement
        data.stubMarshalRead.add("VkAccelerationStructureGeometryTrianglesDataKHR"); // :TODO: how to know if it is deviceAddress or hostAddress
        data.stubMarshalRead.add("VkImportMemoryHostPointerInfoEXT");
        data.stubMarshalRead.add("VkAccelerationStructureGeometryAabbsDataKHR"); // :TODO: how to know if it is deviceAddress or hostAddress
        data.stubMarshalRead.add("VkAccelerationStructureGeometryInstancesDataKHR"); // :TODO: how to know if it is deviceAddress or hostAddress
        data.stubMarshalRead.add("VkAccelerationStructureBuildGeometryInfoKHR"); // :TODO: how to know if it is deviceAddress or hostAddress
        data.stubMarshalRead.add("VkAccelerationStructureGeometryMotionTrianglesDataNV"); // :TODO: how to know if it is deviceAddress or hostAddress
        data.stubMarshalRead.add("VkCopyAccelerationStructureToMemoryInfoKHR"); // :TODO: how to know if it is deviceAddress or hostAddress
        data.stubMarshalRead.add("VkCopyMemoryToAccelerationStructureInfoKHR"); // :TODO: how to know if it is deviceAddress or hostAddress
        data.stubMarshalRead.add("VkCopyMicromapToMemoryInfoEXT"); // :TODO: how to know if it is deviceAddress or hostAddress
        data.stubMarshalRead.add("VkCopyMemoryToMicromapInfoEXT"); // :TODO: how to know if it is deviceAddress or hostAddress

        data.manuallyHandledFunctions.add("vkCreateXlibSurfaceKHR");
        data.manuallyHandledFunctions.add("vkGetPhysicalDeviceXlibPresentationSupportKHR");
        data.manuallyHandledFunctions.add("vkCreateInstance");
        data.manuallyHandledFunctions.add("vkGetDeviceProcAddr");
        data.manuallyHandledFunctions.add("vkEnumerateInstanceExtensionProperties");
        //data.manuallyHandledFunctions.add("vkDestroyInstance");
        data.manuallyHandledFunctions.add("vkGetInstanceProcAddr");

        data.ignoreStructTypes.add("VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR");
        removeUnsupportedTypes(data);
        postParseParams(data); // will set isPointer, needed by setTypeEmulatedSizes
        postParseTypes(data);
        setTypeEmulatedSizes(data);
    }

    static void removeUnsupportedTypes(VkData data) {
        HashSet<VkType> todoDelete = new HashSet<>();

        for (VkType vkType : data.orderedTypes) {
            if (vkType.requires == null) {
                continue;
            }
            if (vkType.requires.equals("windows.h")) {
                todoDelete.add(vkType);
            } else if (vkType.requires.equals("wayland-client.h")) {
                todoDelete.add(vkType);
            } else if (vkType.requires.equals("nvscibuf.h")) {
                todoDelete.add(vkType);
            } else if (vkType.requires.equals("nvscisync.h")) {
                todoDelete.add(vkType);
            } else if (vkType.requires.equals("screen/screen.h")) {
                todoDelete.add(vkType);
            } else if (vkType.requires.equals("xcb/xcb.h")) {
                todoDelete.add(vkType);
            } else if (vkType.requires.equals("X11/extensions/Xrandr.h")) {
                todoDelete.add(vkType);
            } else if (vkType.requires.equals("directfb.h")) {
                todoDelete.add(vkType);
            } else if (vkType.requires.equals("zircon/types.h")) {
                todoDelete.add(vkType);
            } else if (vkType.requires.equals("ggp_c/vulkan_types.h")) {
                todoDelete.add(vkType);
            }
        }
        while (true) {
            boolean found = false;
            for (VkType vkType : data.orderedTypes) {
                for (VkParam vkParam : vkType.members) {
                    if (todoDelete.contains(vkParam.paramType) && !todoDelete.contains(vkType)) {
                        todoDelete.add(vkType);
                        found = true;
                        break;
                    }
                }
            }
            if (!found) {
                break;
            }
        }
        HashSet<VkFunction> deleteFunctions = new HashSet<>();
        for (VkType vkType : todoDelete) {
            data.types.remove(vkType.name);
            data.orderedTypes.remove(vkType);
            for (VkFunction vkFunction : data.functions) {
                for (VkParam vkParam : vkFunction.params) {
                    if (vkParam.paramType == vkType) {
                        deleteFunctions.add(vkFunction);
                        break;
                    }
                }
            }
        }
        for (VkFunction vkFunction : deleteFunctions) {
            data.functions.remove(vkFunction);
        }
    }
    static VkParam addMember(VkData data, VkType parent, String type, String name) {
        VkParam param = new VkParam();
        param.paramType = data.types.get(type);
        param.name = name;
        parent.members.add(param);
        return param;
    }

    static VkParam addMember(VkData data, VkType parent, String type, String name, boolean isPointer, boolean isConst) {
        VkParam param = new VkParam();
        param.paramType = data.types.get(type);
        param.name = name;
        param.isPointer = isPointer;
        param.isConst = isConst;
        parent.members.add(param);
        return param;
    }

    static VkType getType(VkData data, String name) {
        VkType vkType = data.types.get(name);
        if (vkType == null) {
            vkType = new VkType();
            vkType.name = name;
            data.types.put(name, vkType);
            data.orderedTypes.add(vkType);
        }
        return vkType;
    }

    static void updateStdVideoFields(VkData data) {
        VkType vkType = getType(data, "StdVideoEncodeH264SliceHeaderFlags");
        vkType.sizeof = 4;

        vkType = getType(data, "StdVideoH264SliceType");
        vkType.sizeof = 4;

        vkType = getType(data, "StdVideoH264CabacInitIdc");
        vkType.sizeof = 4;

        vkType = getType(data, "StdVideoH264DisableDeblockingFilterIdc");
        vkType.sizeof = 4;

        /*
        typedef struct StdVideoEncodeH264WeightTableFlags {
            uint32_t    luma_weight_l0_flag;
            uint32_t    chroma_weight_l0_flag;
            uint32_t    luma_weight_l1_flag;
            uint32_t    chroma_weight_l1_flag;
        } StdVideoEncodeH264WeightTableFlags;
         */
        vkType = getType(data, "StdVideoEncodeH264WeightTableFlags");
        vkType.setCategory("struct");
        addMember(data, vkType, "uint32_t", "luma_weight_l0_flag");
        addMember(data, vkType, "uint32_t", "chroma_weight_l0_flag");
        addMember(data, vkType, "uint32_t", "luma_weight_l1_flag");
        addMember(data, vkType, "uint32_t", "chroma_weight_l1_flag");

        /*
        typedef struct StdVideoEncodeH264WeightTable {
            StdVideoEncodeH264WeightTableFlags    flags;
            uint8_t                               luma_log2_weight_denom;
            uint8_t                               chroma_log2_weight_denom;
            int8_t                                luma_weight_l0[STD_VIDEO_H264_MAX_NUM_LIST_REF];
            int8_t                                luma_offset_l0[STD_VIDEO_H264_MAX_NUM_LIST_REF];
            int8_t                                chroma_weight_l0[STD_VIDEO_H264_MAX_NUM_LIST_REF][STD_VIDEO_H264_MAX_CHROMA_PLANES];
            int8_t                                chroma_offset_l0[STD_VIDEO_H264_MAX_NUM_LIST_REF][STD_VIDEO_H264_MAX_CHROMA_PLANES];
            int8_t                                luma_weight_l1[STD_VIDEO_H264_MAX_NUM_LIST_REF];
            int8_t                                luma_offset_l1[STD_VIDEO_H264_MAX_NUM_LIST_REF];
            int8_t                                chroma_weight_l1[STD_VIDEO_H264_MAX_NUM_LIST_REF][STD_VIDEO_H264_MAX_CHROMA_PLANES];
            int8_t                                chroma_offset_l1[STD_VIDEO_H264_MAX_NUM_LIST_REF][STD_VIDEO_H264_MAX_CHROMA_PLANES];
        } StdVideoEncodeH264WeightTable;
         */
        vkType = getType(data, "StdVideoEncodeH264WeightTable");
        vkType.setCategory("struct");
        addMember(data, vkType, "StdVideoEncodeH264WeightTableFlags", "flags");
        addMember(data, vkType, "uint8_t", "luma_log2_weight_denom");
        addMember(data, vkType, "uint8_t", "chroma_log2_weight_denom");
        addMember(data, vkType, "int8_t", "luma_weight_l0").arrayLen = 32;
        addMember(data, vkType, "int8_t", "luma_offset_l0").arrayLen = 32;
        addMember(data, vkType, "int8_t", "chroma_weight_l0").arrayLen = 64; // 32x2
        addMember(data, vkType, "int8_t", "chroma_offset_l0").arrayLen = 64; // 32x2
        addMember(data, vkType, "int8_t", "luma_weight_l1").arrayLen = 32;
        addMember(data, vkType, "int8_t", "luma_offset_l1").arrayLen = 32;
        addMember(data, vkType, "int8_t", "chroma_weight_l1").arrayLen = 64; // 32x2
        addMember(data, vkType, "int8_t", "chroma_offset_l1").arrayLen = 64; // 32x2

        /*
        typedef struct StdVideoEncodeH264SliceHeader {
            StdVideoEncodeH264SliceHeaderFlags        flags;
            uint32_t                                  first_mb_in_slice;
            StdVideoH264SliceType                     slice_type;
            int8_t                                    slice_alpha_c0_offset_div2;
            int8_t                                    slice_beta_offset_div2;
            int8_t                                    slice_qp_delta;
            uint8_t                                   reserved1;
            StdVideoH264CabacInitIdc                  cabac_init_idc;
            StdVideoH264DisableDeblockingFilterIdc    disable_deblocking_filter_idc;
            const StdVideoEncodeH264WeightTable*      pWeightTable;
        } StdVideoEncodeH264SliceHeader;
        */

        vkType = getType(data, "StdVideoEncodeH264SliceHeader");
        vkType.setCategory("struct");
        addMember(data, vkType, "StdVideoEncodeH264SliceHeaderFlags","flags");
        addMember(data, vkType, "uint32_t","first_mb_in_slice");
        addMember(data, vkType, "StdVideoH264SliceType","slice_type");
        addMember(data, vkType, "int8_t", "slice_alpha_c0_offset_div2");
        addMember(data, vkType, "int8_t", "slice_beta_offset_div2");
        addMember(data, vkType, "int8_t", "slice_qp_delta");
        addMember(data, vkType, "uint8_t", "reserved1");
        addMember(data, vkType, "StdVideoH264CabacInitIdc", "cabac_init_idc");
        addMember(data, vkType, "StdVideoH264DisableDeblockingFilterIdc", "disable_deblocking_filter_idc");
        addMember(data, vkType, "StdVideoEncodeH264WeightTable", "pWeightTable", true, true).arrayLen = 1; // :TODO: I'm assuming 1

        vkType = getType(data, "StdVideoH264LevelIdc");
        vkType.sizeof = 4;

        vkType = getType(data, "StdVideoH264ProfileIdc");
        vkType.sizeof = 4;

        vkType = getType(data, "StdVideoH265ProfileIdc");
        vkType.sizeof = 4;

        vkType = getType(data, "StdVideoDecodeH264ReferenceInfoFlags");
        vkType.sizeof = 4;

        vkType = getType(data, "StdVideoH265SubLayerHrdParameters");
        /*
        typedef struct StdVideoH265SubLayerHrdParameters {
            uint32_t    bit_rate_value_minus1[STD_VIDEO_H265_CPB_CNT_LIST_SIZE];
            uint32_t    cpb_size_value_minus1[STD_VIDEO_H265_CPB_CNT_LIST_SIZE];
            uint32_t    cpb_size_du_value_minus1[STD_VIDEO_H265_CPB_CNT_LIST_SIZE];
            uint32_t    bit_rate_du_value_minus1[STD_VIDEO_H265_CPB_CNT_LIST_SIZE];
            uint32_t    cbr_flag;
        } StdVideoH265SubLayerHrdParameters;
        */
        vkType.setCategory("struct");
        addMember(data, vkType, "uint32_t", "bit_rate_value_minus1").arrayLen = 32;
        addMember(data, vkType, "uint32_t", "cpb_size_value_minus1").arrayLen = 32;
        addMember(data, vkType, "uint32_t", "cpb_size_du_value_minus1").arrayLen = 32;
        addMember(data, vkType, "uint32_t", "bit_rate_du_value_minus1").arrayLen = 32;
        addMember(data, vkType, "uint32_t", "cbr_flag");

        vkType = getType(data, "StdVideoDecodeH264ReferenceInfo");
        /*
        typedef struct StdVideoDecodeH264ReferenceInfo {
            StdVideoDecodeH264ReferenceInfoFlags    flags;
            uint16_t                                FrameNum;
            uint16_t                                reserved;
            int32_t                                 PicOrderCnt[STD_VIDEO_DECODE_H264_FIELD_ORDER_COUNT_LIST_SIZE];
        } StdVideoDecodeH264ReferenceInfo;
        */
        vkType.setCategory("struct");
        addMember(data, vkType, "StdVideoDecodeH264ReferenceInfoFlags", "flags");
        addMember(data, vkType, "uint16_t", "FrameNum");
        addMember(data, vkType, "uint16_t", "reserved");
        addMember(data, vkType, "int32_t", "PicOrderCnt").arrayLen = 2;

        vkType = getType(data, "StdVideoH265DecPicBufMgr");
        /*
        typedef struct StdVideoH265DecPicBufMgr {
            uint32_t    max_latency_increase_plus1[STD_VIDEO_H265_SUBLAYERS_LIST_SIZE];
            uint8_t     max_dec_pic_buffering_minus1[STD_VIDEO_H265_SUBLAYERS_LIST_SIZE];
            uint8_t     max_num_reorder_pics[STD_VIDEO_H265_SUBLAYERS_LIST_SIZE];
        } StdVideoH265DecPicBufMgr;
         */
        vkType.setCategory("struct");
        addMember(data, vkType, "uint32_t", "max_latency_increase_plus1").arrayLen = 7;
        addMember(data, vkType, "uint8_t", "max_dec_pic_buffering_minus1").arrayLen = 7;
        addMember(data, vkType, "uint8_t", "max_num_reorder_pics").arrayLen = 7;

        vkType = getType(data, "StdVideoH265HrdFlags");
        vkType.sizeof = 4;

        vkType = getType(data, "StdVideoH265HrdParameters");
        /*
        typedef struct StdVideoH265HrdParameters {
            StdVideoH265HrdFlags                        flags;
            uint8_t                                     tick_divisor_minus2;
            uint8_t                                     du_cpb_removal_delay_increment_length_minus1;
            uint8_t                                     dpb_output_delay_du_length_minus1;
            uint8_t                                     bit_rate_scale;
            uint8_t                                     cpb_size_scale;
            uint8_t                                     cpb_size_du_scale;
            uint8_t                                     initial_cpb_removal_delay_length_minus1;
            uint8_t                                     au_cpb_removal_delay_length_minus1;
            uint8_t                                     dpb_output_delay_length_minus1;
            uint8_t                                     cpb_cnt_minus1[STD_VIDEO_H265_SUBLAYERS_LIST_SIZE];
            uint16_t                                    elemental_duration_in_tc_minus1[STD_VIDEO_H265_SUBLAYERS_LIST_SIZE];
            uint16_t                                    reserved[3];
            const StdVideoH265SubLayerHrdParameters*    pSubLayerHrdParametersNal;
            const StdVideoH265SubLayerHrdParameters*    pSubLayerHrdParametersVcl;
        } StdVideoH265HrdParameters;
         */
        vkType.setCategory("struct");
        addMember(data, vkType, "StdVideoH265HrdFlags", "flags");
        addMember(data, vkType, "uint8_t", "tick_divisor_minus2");
        addMember(data, vkType, "uint8_t", "du_cpb_removal_delay_increment_length_minus1");
        addMember(data, vkType, "uint8_t", "dpb_output_delay_du_length_minus1");
        addMember(data, vkType, "uint8_t", "bit_rate_scale");
        addMember(data, vkType, "uint8_t", "cpb_size_scale");
        addMember(data, vkType, "uint8_t", "cpb_size_du_scale");
        addMember(data, vkType, "uint8_t", "initial_cpb_removal_delay_length_minus1");
        addMember(data, vkType, "uint8_t", "au_cpb_removal_delay_length_minus1");
        addMember(data, vkType, "uint8_t", "dpb_output_delay_length_minus1");
        addMember(data, vkType, "uint8_t", "cpb_cnt_minus1").arrayLen = 7;
        addMember(data, vkType, "uint16_t", "elemental_duration_in_tc_minus1").arrayLen = 7;
        addMember(data, vkType, "uint16_t", "reserved").arrayLen = 3;
        addMember(data, vkType, "StdVideoH265SubLayerHrdParameters", "pSubLayerHrdParametersNal", true, true);
        addMember(data, vkType, "StdVideoH265SubLayerHrdParameters", "pSubLayerHrdParametersVcl", true, true);

        vkType = getType(data, "StdVideoH265VpsFlags");
        vkType.sizeof = 4;

        vkType = getType(data, "StdVideoH265ProfileTierLevelFlags");
        vkType.sizeof = 4;

        vkType = getType(data, "StdVideoH265ProfileIdc");
        vkType.sizeof = 4;

        vkType = getType(data, "StdVideoH265LevelIdc");
        vkType.sizeof = 4;

        vkType = getType(data, "StdVideoH265ProfileTierLevel");
        /*
        typedef struct StdVideoH265ProfileTierLevel {
            StdVideoH265ProfileTierLevelFlags    flags;
            StdVideoH265ProfileIdc               general_profile_idc;
            StdVideoH265LevelIdc                 general_level_idc;
        } StdVideoH265ProfileTierLevel;
        */
        vkType.setCategory("struct");
        addMember(data, vkType, "StdVideoH265ProfileTierLevelFlags", "flags");
        addMember(data, vkType, "StdVideoH265ProfileIdc", "general_profile_idc");
        addMember(data, vkType, "StdVideoH265LevelIdc", "general_level_idc");

        vkType = getType(data, "StdVideoH265VideoParameterSet");
        /*
        typedef struct StdVideoH265VideoParameterSet {
            StdVideoH265VpsFlags                   flags;
            uint8_t                                vps_video_parameter_set_id;
            uint8_t                                vps_max_sub_layers_minus1;
            uint8_t                                reserved1;
            uint8_t                                reserved2;
            uint32_t                               vps_num_units_in_tick;
            uint32_t                               vps_time_scale;
            uint32_t                               vps_num_ticks_poc_diff_one_minus1;
            uint32_t                               reserved3;
            const StdVideoH265DecPicBufMgr*        pDecPicBufMgr;
            const StdVideoH265HrdParameters*       pHrdParameters;
            const StdVideoH265ProfileTierLevel*    pProfileTierLevel;
        } StdVideoH265VideoParameterSet;
        */
        vkType.setCategory("struct");
        addMember(data, vkType, "StdVideoH265VpsFlags", "flags");
        addMember(data, vkType, "uint8_t", "vps_video_parameter_set_id");
        addMember(data, vkType, "uint8_t", "vps_max_sub_layers_minus1");
        addMember(data, vkType, "uint8_t", "reserved1");
        addMember(data, vkType, "uint8_t", "reserved2");
        addMember(data, vkType, "uint32_t", "vps_num_units_in_tick");
        addMember(data, vkType, "uint32_t", "vps_time_scale");
        addMember(data, vkType, "uint32_t", "vps_num_ticks_poc_diff_one_minus1");
        addMember(data, vkType, "uint32_t", "reserved3");
        addMember(data, vkType, "StdVideoH265DecPicBufMgr", "pDecPicBufMgr", true, true);
        addMember(data, vkType, "StdVideoH265HrdParameters", "pHrdParameters", true, true);
        addMember(data, vkType, "StdVideoH265ProfileTierLevel", "pProfileTierLevel", true, true);

        vkType = getType(data, "StdVideoH265ScalingLists");
        /*
        typedef struct StdVideoH265ScalingLists {
            uint8_t    ScalingList4x4[STD_VIDEO_H265_SCALING_LIST_4X4_NUM_LISTS][STD_VIDEO_H265_SCALING_LIST_4X4_NUM_ELEMENTS];
            uint8_t    ScalingList8x8[STD_VIDEO_H265_SCALING_LIST_8X8_NUM_LISTS][STD_VIDEO_H265_SCALING_LIST_8X8_NUM_ELEMENTS];
            uint8_t    ScalingList16x16[STD_VIDEO_H265_SCALING_LIST_16X16_NUM_LISTS][STD_VIDEO_H265_SCALING_LIST_16X16_NUM_ELEMENTS];
            uint8_t    ScalingList32x32[STD_VIDEO_H265_SCALING_LIST_32X32_NUM_LISTS][STD_VIDEO_H265_SCALING_LIST_32X32_NUM_ELEMENTS];
            uint8_t    ScalingListDCCoef16x16[STD_VIDEO_H265_SCALING_LIST_16X16_NUM_LISTS];
            uint8_t    ScalingListDCCoef32x32[STD_VIDEO_H265_SCALING_LIST_32X32_NUM_LISTS];
        } StdVideoH265ScalingLists;
        */
        vkType.setCategory("struct");
        addMember(data, vkType, "uint8_t", "ScalingList4x4").arrayLen = 96; // 6x16
        addMember(data, vkType, "uint8_t", "ScalingList8x8").arrayLen = 384; // 6x64
        addMember(data, vkType, "uint8_t", "ScalingList16x16").arrayLen = 384; // 6x64
        addMember(data, vkType, "uint8_t", "ScalingList32x32").arrayLen = 128; // 2x64
        addMember(data, vkType, "uint8_t", "ScalingListDCCoef16x16").arrayLen = 6;
        addMember(data, vkType, "uint8_t", "ScalingListDCCoef32x32").arrayLen = 2;

        vkType = getType(data, "StdVideoH265ShortTermRefPicSetFlags");
        vkType.sizeof = 4;

        vkType = getType(data, "StdVideoH265ShortTermRefPicSet");
        /*
        typedef struct StdVideoH265ShortTermRefPicSet {
            StdVideoH265ShortTermRefPicSetFlags    flags;
            uint32_t                               delta_idx_minus1;
            uint16_t                               use_delta_flag;
            uint16_t                               abs_delta_rps_minus1;
            uint16_t                               used_by_curr_pic_flag;
            uint16_t                               used_by_curr_pic_s0_flag;
            uint16_t                               used_by_curr_pic_s1_flag;
            uint16_t                               reserved1;
            uint8_t                                reserved2;
            uint8_t                                reserved3;
            uint8_t                                num_negative_pics;
            uint8_t                                num_positive_pics;
            uint16_t                               delta_poc_s0_minus1[STD_VIDEO_H265_MAX_DPB_SIZE];
            uint16_t                               delta_poc_s1_minus1[STD_VIDEO_H265_MAX_DPB_SIZE];
        } StdVideoH265ShortTermRefPicSet;
        */
        vkType.setCategory("struct");
        addMember(data, vkType, "StdVideoH265ShortTermRefPicSetFlags", "flags");
        addMember(data, vkType, "uint32_t", "delta_idx_minus1");
        addMember(data, vkType, "uint16_t", "use_delta_flag");
        addMember(data, vkType, "uint16_t", "abs_delta_rps_minus1");
        addMember(data, vkType, "uint16_t", "used_by_curr_pic_flag");
        addMember(data, vkType, "uint16_t", "used_by_curr_pic_s0_flag");
        addMember(data, vkType, "uint16_t", "used_by_curr_pic_s1_flag");
        addMember(data, vkType, "uint16_t", "reserved1");
        addMember(data, vkType, "uint8_t", "reserved2");
        addMember(data, vkType, "uint8_t", "reserved3");
        addMember(data, vkType, "uint8_t", "num_negative_pics");
        addMember(data, vkType, "uint8_t", "num_positive_pics");
        addMember(data, vkType, "uint16_t", "delta_poc_s0_minus1").arrayLen = 16;
        addMember(data, vkType, "uint16_t", "delta_poc_s1_minus1").arrayLen = 16;

        vkType = getType(data, "StdVideoH265LongTermRefPicsSps");
        /*
        typedef struct StdVideoH265LongTermRefPicsSps {
            uint32_t    used_by_curr_pic_lt_sps_flag;
            uint32_t    lt_ref_pic_poc_lsb_sps[STD_VIDEO_H265_MAX_LONG_TERM_REF_PICS_SPS];
        } StdVideoH265LongTermRefPicsSps;
        */
        vkType.setCategory("struct");
        addMember(data, vkType, "uint32_t", "used_by_curr_pic_lt_sps_flag");
        addMember(data, vkType, "uint32_t", "lt_ref_pic_poc_lsb_sps").arrayLen = 32;

        vkType = getType(data, "StdVideoH265PredictorPaletteEntries");
        /*
        typedef struct StdVideoH265PredictorPaletteEntries {
            uint16_t    PredictorPaletteEntries[STD_VIDEO_H265_PREDICTOR_PALETTE_COMPONENTS_LIST_SIZE][STD_VIDEO_H265_PREDICTOR_PALETTE_COMP_ENTRIES_LIST_SIZE];
        } StdVideoH265PredictorPaletteEntries;
        */
        vkType.setCategory("struct");
        addMember(data, vkType, "uint16_t", "PredictorPaletteEntries").arrayLen = 384; // 3x128

        vkType = getType(data, "StdVideoH265SpsVuiFlags");
        vkType.sizeof = 4;

        vkType = getType(data, "StdVideoH265AspectRatioIdc");
        vkType.sizeof = 4;

        vkType = getType(data, "StdVideoH265SequenceParameterSetVui");
        /*
        typedef struct StdVideoH265SequenceParameterSetVui {
            StdVideoH265SpsVuiFlags             flags;
            StdVideoH265AspectRatioIdc          aspect_ratio_idc;
            uint16_t                            sar_width;
            uint16_t                            sar_height;
            uint8_t                             video_format;
            uint8_t                             colour_primaries;
            uint8_t                             transfer_characteristics;
            uint8_t                             matrix_coeffs;
            uint8_t                             chroma_sample_loc_type_top_field;
            uint8_t                             chroma_sample_loc_type_bottom_field;
            uint8_t                             reserved1;
            uint8_t                             reserved2;
            uint16_t                            def_disp_win_left_offset;
            uint16_t                            def_disp_win_right_offset;
            uint16_t                            def_disp_win_top_offset;
            uint16_t                            def_disp_win_bottom_offset;
            uint32_t                            vui_num_units_in_tick;
            uint32_t                            vui_time_scale;
            uint32_t                            vui_num_ticks_poc_diff_one_minus1;
            uint16_t                            min_spatial_segmentation_idc;
            uint16_t                            reserved3;
            uint8_t                             max_bytes_per_pic_denom;
            uint8_t                             max_bits_per_min_cu_denom;
            uint8_t                             log2_max_mv_length_horizontal;
            uint8_t                             log2_max_mv_length_vertical;
            const StdVideoH265HrdParameters*    pHrdParameters;
        } StdVideoH265SequenceParameterSetVui;
        */
        vkType.setCategory("struct");
        addMember(data, vkType, "StdVideoH265SpsVuiFlags", "flags");
        addMember(data, vkType, "StdVideoH265AspectRatioIdc", "aspect_ratio_idc");
        addMember(data, vkType, "uint16_t", "sar_width");
        addMember(data, vkType, "uint16_t", "sar_height");
        addMember(data, vkType, "uint8_t", "video_format");
        addMember(data, vkType, "uint8_t", "colour_primaries");
        addMember(data, vkType, "uint8_t", "transfer_characteristics");
        addMember(data, vkType, "uint8_t", "matrix_coeffs");
        addMember(data, vkType, "uint8_t", "chroma_sample_loc_type_top_field");
        addMember(data, vkType, "uint8_t", "chroma_sample_loc_type_bottom_field");
        addMember(data, vkType, "uint8_t", "reserved1");
        addMember(data, vkType, "uint8_t", "reserved2");
        addMember(data, vkType, "uint16_t", "def_disp_win_left_offset");
        addMember(data, vkType, "uint16_t", "def_disp_win_right_offset");
        addMember(data, vkType, "uint16_t", "def_disp_win_top_offset");
        addMember(data, vkType, "uint16_t", "def_disp_win_bottom_offset");
        addMember(data, vkType, "uint32_t", "vui_num_units_in_tick");
        addMember(data, vkType, "uint32_t", "vui_time_scale");
        addMember(data, vkType, "uint32_t", "vui_num_ticks_poc_diff_one_minus1");
        addMember(data, vkType, "uint16_t", "min_spatial_segmentation_idc");
        addMember(data, vkType, "uint16_t", "reserved3");
        addMember(data, vkType, "uint8_t", "max_bytes_per_pic_denom");
        addMember(data, vkType, "uint8_t", "max_bits_per_min_cu_denom");
        addMember(data, vkType, "uint8_t", "log2_max_mv_length_horizontal");
        addMember(data, vkType, "uint8_t", "log2_max_mv_length_vertical");
        addMember(data, vkType, "StdVideoH265HrdParameters", "pHrdParameters", true, true);

        vkType = getType(data, "StdVideoH265SpsFlags");
        vkType.sizeof = 4;

        vkType = getType(data, "StdVideoH265ChromaFormatIdc");
        vkType.sizeof = 4;

        vkType = getType(data, "StdVideoH265SequenceParameterSet");
        /*
        typedef struct StdVideoH265SequenceParameterSet {
            StdVideoH265SpsFlags                          flags;
            StdVideoH265ChromaFormatIdc                   chroma_format_idc;
            uint32_t                                      pic_width_in_luma_samples;
            uint32_t                                      pic_height_in_luma_samples;
            uint8_t                                       sps_video_parameter_set_id;
            uint8_t                                       sps_max_sub_layers_minus1;
            uint8_t                                       sps_seq_parameter_set_id;
            uint8_t                                       bit_depth_luma_minus8;
            uint8_t                                       bit_depth_chroma_minus8;
            uint8_t                                       log2_max_pic_order_cnt_lsb_minus4;
            uint8_t                                       log2_min_luma_coding_block_size_minus3;
            uint8_t                                       log2_diff_max_min_luma_coding_block_size;
            uint8_t                                       log2_min_luma_transform_block_size_minus2;
            uint8_t                                       log2_diff_max_min_luma_transform_block_size;
            uint8_t                                       max_transform_hierarchy_depth_inter;
            uint8_t                                       max_transform_hierarchy_depth_intra;
            uint8_t                                       num_short_term_ref_pic_sets;
            uint8_t                                       num_long_term_ref_pics_sps;
            uint8_t                                       pcm_sample_bit_depth_luma_minus1;
            uint8_t                                       pcm_sample_bit_depth_chroma_minus1;
            uint8_t                                       log2_min_pcm_luma_coding_block_size_minus3;
            uint8_t                                       log2_diff_max_min_pcm_luma_coding_block_size;
            uint8_t                                       reserved1;
            uint8_t                                       reserved2;
            uint8_t                                       palette_max_size;
            uint8_t                                       delta_palette_max_predictor_size;
            uint8_t                                       motion_vector_resolution_control_idc;
            uint8_t                                       sps_num_palette_predictor_initializers_minus1;
            uint32_t                                      conf_win_left_offset;
            uint32_t                                      conf_win_right_offset;
            uint32_t                                      conf_win_top_offset;
            uint32_t                                      conf_win_bottom_offset;
            const StdVideoH265ProfileTierLevel*           pProfileTierLevel;
            const StdVideoH265DecPicBufMgr*               pDecPicBufMgr;
            const StdVideoH265ScalingLists*               pScalingLists;
            const StdVideoH265ShortTermRefPicSet*         pShortTermRefPicSet;
            const StdVideoH265LongTermRefPicsSps*         pLongTermRefPicsSps;
            const StdVideoH265SequenceParameterSetVui*    pSequenceParameterSetVui;
            const StdVideoH265PredictorPaletteEntries*    pPredictorPaletteEntries;
        } StdVideoH265SequenceParameterSet;
        */
        vkType.setCategory("struct");
        addMember(data, vkType, "StdVideoH265SpsFlags", "flags");
        addMember(data, vkType, "StdVideoH265ChromaFormatIdc", "chroma_format_idc");
        addMember(data, vkType, "uint32_t", "pic_width_in_luma_samples");
        addMember(data, vkType, "uint32_t", "pic_height_in_luma_samples");
        addMember(data, vkType, "uint8_t", "sps_video_parameter_set_id");
        addMember(data, vkType, "uint8_t", "sps_max_sub_layers_minus1");
        addMember(data, vkType, "uint8_t", "sps_seq_parameter_set_id");
        addMember(data, vkType, "uint8_t", "bit_depth_luma_minus8");
        addMember(data, vkType, "uint8_t", "bit_depth_chroma_minus8");
        addMember(data, vkType, "uint8_t", "log2_max_pic_order_cnt_lsb_minus4");
        addMember(data, vkType, "uint8_t", "log2_min_luma_coding_block_size_minus3");
        addMember(data, vkType, "uint8_t", "log2_diff_max_min_luma_coding_block_size");
        addMember(data, vkType, "uint8_t", "log2_min_luma_transform_block_size_minus2");
        addMember(data, vkType, "uint8_t", "log2_diff_max_min_luma_transform_block_size");
        addMember(data, vkType, "uint8_t", "max_transform_hierarchy_depth_inter");
        addMember(data, vkType, "uint8_t", "max_transform_hierarchy_depth_intra");
        addMember(data, vkType, "uint8_t", "num_short_term_ref_pic_sets");
        addMember(data, vkType, "uint8_t", "num_long_term_ref_pics_sps");
        addMember(data, vkType, "uint8_t", "pcm_sample_bit_depth_luma_minus1");
        addMember(data, vkType, "uint8_t", "pcm_sample_bit_depth_chroma_minus1");
        addMember(data, vkType, "uint8_t", "log2_min_pcm_luma_coding_block_size_minus3");
        addMember(data, vkType, "uint8_t", "log2_diff_max_min_pcm_luma_coding_block_size");
        addMember(data, vkType, "uint8_t", "reserved1");
        addMember(data, vkType, "uint8_t", "reserved2");
        addMember(data, vkType, "uint8_t", "palette_max_size");
        addMember(data, vkType, "uint8_t", "delta_palette_max_predictor_size");
        addMember(data, vkType, "uint8_t", "motion_vector_resolution_control_idc");
        addMember(data, vkType, "uint8_t", "sps_num_palette_predictor_initializers_minus1");
        addMember(data, vkType, "uint32_t", "conf_win_left_offset");
        addMember(data, vkType, "uint32_t", "conf_win_right_offset");
        addMember(data, vkType, "uint32_t", "conf_win_top_offset");
        addMember(data, vkType, "uint32_t", "conf_win_bottom_offset");
        addMember(data, vkType, "StdVideoH265ProfileTierLevel", "pProfileTierLevel", true, true);
        addMember(data, vkType, "StdVideoH265DecPicBufMgr", "pDecPicBufMgr", true, true);
        addMember(data, vkType, "StdVideoH265ScalingLists", "pScalingLists", true, true);
        addMember(data, vkType, "StdVideoH265ShortTermRefPicSet", "pShortTermRefPicSet", true, true);
        addMember(data, vkType, "StdVideoH265LongTermRefPicsSps", "pLongTermRefPicsSps", true, true);
        addMember(data, vkType, "StdVideoH265SequenceParameterSetVui", "pSequenceParameterSetVui", true, true);
        addMember(data, vkType, "StdVideoH265PredictorPaletteEntries", "pPredictorPaletteEntries", true, true);

        vkType = getType(data, "StdVideoH265PpsFlags");
        vkType.sizeof = 4;

        vkType = getType(data, "StdVideoH265PictureParameterSet");
        /*
        typedef struct StdVideoH265PictureParameterSet {
            StdVideoH265PpsFlags                          flags;
            uint8_t                                       pps_pic_parameter_set_id;
            uint8_t                                       pps_seq_parameter_set_id;
            uint8_t                                       sps_video_parameter_set_id;
            uint8_t                                       num_extra_slice_header_bits;
            uint8_t                                       num_ref_idx_l0_default_active_minus1;
            uint8_t                                       num_ref_idx_l1_default_active_minus1;
            int8_t                                        init_qp_minus26;
            uint8_t                                       diff_cu_qp_delta_depth;
            int8_t                                        pps_cb_qp_offset;
            int8_t                                        pps_cr_qp_offset;
            int8_t                                        pps_beta_offset_div2;
            int8_t                                        pps_tc_offset_div2;
            uint8_t                                       log2_parallel_merge_level_minus2;
            uint8_t                                       log2_max_transform_skip_block_size_minus2;
            uint8_t                                       diff_cu_chroma_qp_offset_depth;
            uint8_t                                       chroma_qp_offset_list_len_minus1;
            int8_t                                        cb_qp_offset_list[STD_VIDEO_H265_CHROMA_QP_OFFSET_LIST_SIZE];
            int8_t                                        cr_qp_offset_list[STD_VIDEO_H265_CHROMA_QP_OFFSET_LIST_SIZE];
            uint8_t                                       log2_sao_offset_scale_luma;
            uint8_t                                       log2_sao_offset_scale_chroma;
            int8_t                                        pps_act_y_qp_offset_plus5;
            int8_t                                        pps_act_cb_qp_offset_plus5;
            int8_t                                        pps_act_cr_qp_offset_plus3;
            uint8_t                                       pps_num_palette_predictor_initializers;
            uint8_t                                       luma_bit_depth_entry_minus8;
            uint8_t                                       chroma_bit_depth_entry_minus8;
            uint8_t                                       num_tile_columns_minus1;
            uint8_t                                       num_tile_rows_minus1;
            uint8_t                                       reserved1;
            uint8_t                                       reserved2;
            uint16_t                                      column_width_minus1[STD_VIDEO_H265_CHROMA_QP_OFFSET_TILE_COLS_LIST_SIZE];
            uint16_t                                      row_height_minus1[STD_VIDEO_H265_CHROMA_QP_OFFSET_TILE_ROWS_LIST_SIZE];
            uint32_t                                      reserved3;
            const StdVideoH265ScalingLists*               pScalingLists;
            const StdVideoH265PredictorPaletteEntries*    pPredictorPaletteEntries;
        } StdVideoH265PictureParameterSet;
        */
        vkType.setCategory("struct");
        addMember(data, vkType, "StdVideoH265PpsFlags", "flags");
        addMember(data, vkType, "uint8_t", "pps_pic_parameter_set_id");
        addMember(data, vkType, "uint8_t", "pps_seq_parameter_set_id");
        addMember(data, vkType, "uint8_t", "sps_video_parameter_set_id");
        addMember(data, vkType, "uint8_t", "num_extra_slice_header_bits");
        addMember(data, vkType, "uint8_t", "num_ref_idx_l0_default_active_minus1");
        addMember(data, vkType, "uint8_t", "num_ref_idx_l1_default_active_minus1");
        addMember(data, vkType, "int8_t", "init_qp_minus26");
        addMember(data, vkType, "uint8_t", "diff_cu_qp_delta_depth");
        addMember(data, vkType, "int8_t", "pps_cb_qp_offset");
        addMember(data, vkType, "int8_t", "pps_cr_qp_offset");
        addMember(data, vkType, "int8_t", "pps_beta_offset_div2");
        addMember(data, vkType, "int8_t", "pps_tc_offset_div2");
        addMember(data, vkType, "uint8_t", "log2_parallel_merge_level_minus2");
        addMember(data, vkType, "uint8_t", "log2_max_transform_skip_block_size_minus2");
        addMember(data, vkType, "uint8_t", "diff_cu_chroma_qp_offset_depth");
        addMember(data, vkType, "uint8_t", "chroma_qp_offset_list_len_minus1");
        addMember(data, vkType, "int8_t", "cb_qp_offset_list").arrayLen = 6;
        addMember(data, vkType, "int8_t", "cr_qp_offset_list").arrayLen = 6;
        addMember(data, vkType, "uint8_t", "log2_sao_offset_scale_luma");
        addMember(data, vkType, "uint8_t", "log2_sao_offset_scale_chroma");
        addMember(data, vkType, "int8_t", "pps_act_y_qp_offset_plus5");
        addMember(data, vkType, "int8_t", "pps_act_cb_qp_offset_plus5");
        addMember(data, vkType, "int8_t", "pps_act_cr_qp_offset_plus3");
        addMember(data, vkType, "uint8_t", "pps_num_palette_predictor_initializers");
        addMember(data, vkType, "uint8_t", "luma_bit_depth_entry_minus8");
        addMember(data, vkType, "uint8_t", "chroma_bit_depth_entry_minus8");
        addMember(data, vkType, "uint8_t", "num_tile_columns_minus1");
        addMember(data, vkType, "uint8_t", "num_tile_rows_minus1");
        addMember(data, vkType, "uint8_t", "reserved1");
        addMember(data, vkType, "uint8_t", "reserved2");
        addMember(data, vkType, "uint16_t", "column_width_minus1").arrayLen = 19;
        addMember(data, vkType, "uint16_t", "row_height_minus1").arrayLen = 21;
        addMember(data, vkType, "uint32_t", "reserved3");
        addMember(data, vkType, "StdVideoH265ScalingLists", "pScalingLists", true, true);
        addMember(data, vkType, "StdVideoH265PredictorPaletteEntries", "pPredictorPaletteEntries", true, true);

        vkType = getType(data, "StdVideoEncodeH265WeightTableFlags");
         /*
        typedef struct StdVideoEncodeH265WeightTableFlags {
            uint16_t    luma_weight_l0_flag;
            uint16_t    chroma_weight_l0_flag;
            uint16_t    luma_weight_l1_flag;
            uint16_t    chroma_weight_l1_flag;
        } StdVideoEncodeH265WeightTableFlags;
        */
        vkType.setCategory("struct");
        addMember(data, vkType, "uint16_t", "luma_weight_l0_flag");
        addMember(data, vkType, "uint16_t", "chroma_weight_l0_flag");
        addMember(data, vkType, "uint16_t", "luma_weight_l1_flag");
        addMember(data, vkType, "uint16_t", "chroma_weight_l1_flag");

        vkType = getType(data, "StdVideoEncodeH265WeightTable");
        /*
        typedef struct StdVideoEncodeH265WeightTable {
            StdVideoEncodeH265WeightTableFlags    flags;
            uint8_t                               luma_log2_weight_denom;
            int8_t                                delta_chroma_log2_weight_denom;
            int8_t                                delta_luma_weight_l0[STD_VIDEO_H265_MAX_NUM_LIST_REF];
            int8_t                                luma_offset_l0[STD_VIDEO_H265_MAX_NUM_LIST_REF];
            int8_t                                delta_chroma_weight_l0[STD_VIDEO_H265_MAX_NUM_LIST_REF][STD_VIDEO_H265_MAX_CHROMA_PLANES];
            int8_t                                delta_chroma_offset_l0[STD_VIDEO_H265_MAX_NUM_LIST_REF][STD_VIDEO_H265_MAX_CHROMA_PLANES];
            int8_t                                delta_luma_weight_l1[STD_VIDEO_H265_MAX_NUM_LIST_REF];
            int8_t                                luma_offset_l1[STD_VIDEO_H265_MAX_NUM_LIST_REF];
            int8_t                                delta_chroma_weight_l1[STD_VIDEO_H265_MAX_NUM_LIST_REF][STD_VIDEO_H265_MAX_CHROMA_PLANES];
            int8_t                                delta_chroma_offset_l1[STD_VIDEO_H265_MAX_NUM_LIST_REF][STD_VIDEO_H265_MAX_CHROMA_PLANES];
        } StdVideoEncodeH265WeightTable;
        */
        vkType.setCategory("struct");
        addMember(data, vkType, "StdVideoEncodeH265WeightTableFlags", "flags");
        addMember(data, vkType, "uint8_t", "luma_log2_weight_denom");
        addMember(data, vkType, "int8_t", "delta_chroma_log2_weight_denom");
        addMember(data, vkType, "int8_t", "delta_luma_weight_l0").arrayLen = 15;
        addMember(data, vkType, "int8_t", "luma_offset_l0").arrayLen = 15;
        addMember(data, vkType, "int8_t", "delta_chroma_weight_l0").arrayLen = 30; // 15x2
        addMember(data, vkType, "int8_t", "delta_chroma_offset_l0").arrayLen = 30; // 15x2
        addMember(data, vkType, "int8_t", "delta_luma_weight_l1").arrayLen = 15;
        addMember(data, vkType, "int8_t", "luma_offset_l1").arrayLen = 15;
        addMember(data, vkType, "int8_t", "delta_chroma_weight_l1").arrayLen = 30; // 15x2
        addMember(data, vkType, "int8_t", "delta_chroma_offset_l1").arrayLen = 30; // 15x2

        vkType = getType(data, "StdVideoEncodeH265SliceSegmentHeaderFlags");
        vkType.sizeof = 4;

        vkType = getType(data, "StdVideoH265SliceType");
        vkType.sizeof = 4;

        vkType = getType(data, "StdVideoEncodeH265SliceSegmentHeader");
        /*
        typedef struct StdVideoEncodeH265SliceSegmentHeader {
            StdVideoEncodeH265SliceSegmentHeaderFlags    flags;
            StdVideoH265SliceType                        slice_type;
            uint32_t                                     slice_segment_address;
            uint8_t                                      collocated_ref_idx;
            uint8_t                                      MaxNumMergeCand;
            int8_t                                       slice_cb_qp_offset;
            int8_t                                       slice_cr_qp_offset;
            int8_t                                       slice_beta_offset_div2;
            int8_t                                       slice_tc_offset_div2;
            int8_t                                       slice_act_y_qp_offset;
            int8_t                                       slice_act_cb_qp_offset;
            int8_t                                       slice_act_cr_qp_offset;
            int8_t                                       slice_qp_delta;
            uint16_t                                     reserved1;
            const StdVideoEncodeH265WeightTable*         pWeightTable;
        } StdVideoEncodeH265SliceSegmentHeader;
        */
        vkType.setCategory("struct");
        addMember(data, vkType, "StdVideoEncodeH265SliceSegmentHeaderFlags", "flags");
        addMember(data, vkType, "StdVideoH265SliceType", "slice_type");
        addMember(data, vkType, "uint32_t", "slice_segment_address");
        addMember(data, vkType, "uint8_t", "collocated_ref_idx");
        addMember(data, vkType, "uint8_t", "MaxNumMergeCand");
        addMember(data, vkType, "int8_t", "slice_cb_qp_offset");
        addMember(data, vkType, "int8_t", "slice_cr_qp_offset");
        addMember(data, vkType, "int8_t", "slice_beta_offset_div2");
        addMember(data, vkType, "int8_t", "slice_tc_offset_div2");
        addMember(data, vkType, "int8_t", "slice_act_y_qp_offset");
        addMember(data, vkType, "int8_t", "slice_act_cb_qp_offset");
        addMember(data, vkType, "int8_t", "slice_act_cr_qp_offset");
        addMember(data, vkType, "int8_t", "slice_qp_delta");
        addMember(data, vkType, "uint16_t", "reserved1");
        addMember(data, vkType, "StdVideoEncodeH265WeightTable", "pWeightTable", true, true);

        vkType = getType(data, "StdVideoEncodeH265ReferenceListsInfoFlags");
        vkType.sizeof = 4;

        vkType = getType(data, "StdVideoEncodeH265ReferenceListsInfo");
        /*
        typedef struct StdVideoEncodeH265ReferenceListsInfo {
            StdVideoEncodeH265ReferenceListsInfoFlags    flags;
            uint8_t                                      num_ref_idx_l0_active_minus1;
            uint8_t                                      num_ref_idx_l1_active_minus1;
            uint8_t                                      RefPicList0[STD_VIDEO_H265_MAX_NUM_LIST_REF];
            uint8_t                                      RefPicList1[STD_VIDEO_H265_MAX_NUM_LIST_REF];
            uint8_t                                      list_entry_l0[STD_VIDEO_H265_MAX_NUM_LIST_REF];
            uint8_t                                      list_entry_l1[STD_VIDEO_H265_MAX_NUM_LIST_REF];
        } StdVideoEncodeH265ReferenceListsInfo;
        */
        vkType.setCategory("struct");
        addMember(data, vkType, "StdVideoEncodeH265ReferenceListsInfoFlags", "flags");
        addMember(data, vkType, "uint8_t", "num_ref_idx_l0_active_minus1");
        addMember(data, vkType, "uint8_t", "num_ref_idx_l1_active_minus1");
        addMember(data, vkType, "uint8_t", "RefPicList0").arrayLen = 15;
        addMember(data, vkType, "uint8_t", "RefPicList1").arrayLen = 15;
        addMember(data, vkType, "uint8_t", "list_entry_l0").arrayLen = 15;
        addMember(data, vkType, "uint8_t", "list_entry_l1").arrayLen = 15;

        vkType = getType(data, "StdVideoEncodeH265LongTermRefPics");
        /*
        typedef struct StdVideoEncodeH265LongTermRefPics {
            uint8_t     num_long_term_sps;
            uint8_t     num_long_term_pics;
            uint8_t     lt_idx_sps[STD_VIDEO_H265_MAX_LONG_TERM_REF_PICS_SPS];
            uint8_t     poc_lsb_lt[STD_VIDEO_H265_MAX_LONG_TERM_PICS];
            uint16_t    used_by_curr_pic_lt_flag;
            uint8_t     delta_poc_msb_present_flag[STD_VIDEO_H265_MAX_DELTA_POC];
            uint8_t     delta_poc_msb_cycle_lt[STD_VIDEO_H265_MAX_DELTA_POC];
        } StdVideoEncodeH265LongTermRefPics;
        */
        vkType.setCategory("struct");
        addMember(data, vkType, "uint8_t", "num_long_term_sps");
        addMember(data, vkType, "uint8_t", "num_long_term_pics");
        addMember(data, vkType, "uint8_t", "lt_idx_sps").arrayLen = 32;
        addMember(data, vkType, "uint8_t", "poc_lsb_lt").arrayLen = 16;
        addMember(data, vkType, "uint16_t", "used_by_curr_pic_lt_flag");
        addMember(data, vkType, "uint8_t", "delta_poc_msb_present_flag").arrayLen = 48;
        addMember(data, vkType, "uint8_t", "delta_poc_msb_cycle_lt").arrayLen = 48;

        vkType = getType(data, "StdVideoH265PictureType");
        vkType.sizeof = 4;

        vkType = getType(data, "StdVideoEncodeH265PictureInfoFlags");
        vkType.sizeof = 4;

        vkType = getType(data, "StdVideoEncodeH265PictureInfo");
        /*
        typedef struct StdVideoEncodeH265PictureInfo {
            StdVideoEncodeH265PictureInfoFlags             flags;
            StdVideoH265PictureType                        pic_type;
            uint8_t                                        sps_video_parameter_set_id;
            uint8_t                                        pps_seq_parameter_set_id;
            uint8_t                                        pps_pic_parameter_set_id;
            uint8_t                                        short_term_ref_pic_set_idx;
            int32_t                                        PicOrderCntVal;
            uint8_t                                        TemporalId;
            uint8_t                                        reserved1[7];
            const StdVideoEncodeH265ReferenceListsInfo*    pRefLists;
            const StdVideoH265ShortTermRefPicSet*          pShortTermRefPicSet;
            const StdVideoEncodeH265LongTermRefPics*       pLongTermRefPics;
        } StdVideoEncodeH265PictureInfo;
        */
        vkType.setCategory("struct");
        addMember(data, vkType, "StdVideoEncodeH265PictureInfoFlags", "flags");
        addMember(data, vkType, "StdVideoH265PictureType", "pic_type");
        addMember(data, vkType, "uint8_t", "sps_video_parameter_set_id");
        addMember(data, vkType, "uint8_t", "pps_seq_parameter_set_id");
        addMember(data, vkType, "uint8_t", "pps_pic_parameter_set_id");
        addMember(data, vkType, "uint8_t", "short_term_ref_pic_set_idx");
        addMember(data, vkType, "int32_t", "PicOrderCntVal");
        addMember(data, vkType, "uint8_t", "TemporalId");
        addMember(data, vkType, "uint8_t", "reserved1").arrayLen = 7;
        addMember(data, vkType, "StdVideoEncodeH265ReferenceListsInfo", "pRefLists", true, true);
        addMember(data, vkType, "StdVideoH265ShortTermRefPicSet", "pShortTermRefPicSet", true, true);
        addMember(data, vkType, "StdVideoEncodeH265LongTermRefPics", "pLongTermRefPics", true, true);

        vkType = getType(data, "StdVideoDecodeH264PictureInfoFlags");
        vkType.sizeof = 4;

        vkType = getType(data, "StdVideoDecodeH264PictureInfo");
        /*
        typedef struct StdVideoDecodeH264PictureInfo {
            StdVideoDecodeH264PictureInfoFlags    flags;
            uint8_t                               seq_parameter_set_id;
            uint8_t                               pic_parameter_set_id;
            uint8_t                               reserved1;
            uint8_t                               reserved2;
            uint16_t                              frame_num;
            uint16_t                              idr_pic_id;
            int32_t                               PicOrderCnt[STD_VIDEO_DECODE_H264_FIELD_ORDER_COUNT_LIST_SIZE];
        } StdVideoDecodeH264PictureInfo;
        */
        vkType.setCategory("struct");
        addMember(data, vkType, "StdVideoDecodeH264PictureInfoFlags", "flags");
        addMember(data, vkType, "uint8_t", "seq_parameter_set_id");
        addMember(data, vkType, "uint8_t", "pic_parameter_set_id");
        addMember(data, vkType, "uint8_t", "reserved1");
        addMember(data, vkType, "uint8_t", "reserved2");
        addMember(data, vkType, "uint16_t", "frame_num");
        addMember(data, vkType, "uint16_t", "idr_pic_id");
        addMember(data, vkType, "int32_t", "PicOrderCnt").arrayLen = 2;

        vkType = getType(data, "StdVideoH264ScalingLists");
        /*
        typedef struct StdVideoH264ScalingLists {
            uint16_t    scaling_list_present_mask;
            uint16_t    use_default_scaling_matrix_mask;
            uint8_t     ScalingList4x4[STD_VIDEO_H264_SCALING_LIST_4X4_NUM_LISTS][STD_VIDEO_H264_SCALING_LIST_4X4_NUM_ELEMENTS];
            uint8_t     ScalingList8x8[STD_VIDEO_H264_SCALING_LIST_8X8_NUM_LISTS][STD_VIDEO_H264_SCALING_LIST_8X8_NUM_ELEMENTS];
        } StdVideoH264ScalingLists;
        */
        vkType.setCategory("struct");
        addMember(data, vkType, "uint16_t", "scaling_list_present_mask");
        addMember(data, vkType, "uint16_t", "use_default_scaling_matrix_mask");
        addMember(data, vkType, "uint8_t", "ScalingList4x4").arrayLen = 6*16; // 6x16
        addMember(data, vkType, "uint8_t", "ScalingList8x8").arrayLen = 6*64; // 6x64

        vkType = getType(data, "StdVideoH264SpsVuiFlags");
        vkType.sizeof = 4;

        vkType = getType(data, "StdVideoH264AspectRatioIdc");
        vkType.sizeof = 4;

        vkType = getType(data, "StdVideoH264HrdParameters");
        /*
        typedef struct StdVideoH264HrdParameters {
            uint8_t     cpb_cnt_minus1;
            uint8_t     bit_rate_scale;
            uint8_t     cpb_size_scale;
            uint8_t     reserved1;
            uint32_t    bit_rate_value_minus1[STD_VIDEO_H264_CPB_CNT_LIST_SIZE];
            uint32_t    cpb_size_value_minus1[STD_VIDEO_H264_CPB_CNT_LIST_SIZE];
            uint8_t     cbr_flag[STD_VIDEO_H264_CPB_CNT_LIST_SIZE];
            uint32_t    initial_cpb_removal_delay_length_minus1;
            uint32_t    cpb_removal_delay_length_minus1;
            uint32_t    dpb_output_delay_length_minus1;
            uint32_t    time_offset_length;
        } StdVideoH264HrdParameters;
        */
        vkType.setCategory("struct");
        addMember(data, vkType, "uint8_t", "cpb_cnt_minus1");
        addMember(data, vkType, "uint8_t", "bit_rate_scale");
        addMember(data, vkType, "uint8_t", "cpb_size_scale");
        addMember(data, vkType, "uint8_t", "reserved1");
        addMember(data, vkType, "uint32_t", "bit_rate_value_minus1").arrayLen = 32;
        addMember(data, vkType, "uint32_t", "cpb_size_value_minus1").arrayLen = 32;
        addMember(data, vkType, "uint8_t", "cbr_flag").arrayLen = 32;
        addMember(data, vkType, "uint32_t", "initial_cpb_removal_delay_length_minus1");
        addMember(data, vkType, "uint32_t", "cpb_removal_delay_length_minus1");
        addMember(data, vkType, "uint32_t", "dpb_output_delay_length_minus1");
        addMember(data, vkType, "uint32_t", "time_offset_length");

        vkType = getType(data, "StdVideoH264SequenceParameterSetVui");
        /*
        typedef struct StdVideoH264SequenceParameterSetVui {
            StdVideoH264SpsVuiFlags             flags;
            StdVideoH264AspectRatioIdc          aspect_ratio_idc;
            uint16_t                            sar_width;
            uint16_t                            sar_height;
            uint8_t                             video_format;
            uint8_t                             colour_primaries;
            uint8_t                             transfer_characteristics;
            uint8_t                             matrix_coefficients;
            uint32_t                            num_units_in_tick;
            uint32_t                            time_scale;
            uint8_t                             max_num_reorder_frames;
            uint8_t                             max_dec_frame_buffering;
            uint8_t                             chroma_sample_loc_type_top_field;
            uint8_t                             chroma_sample_loc_type_bottom_field;
            uint32_t                            reserved1;
            const StdVideoH264HrdParameters*    pHrdParameters;
        } StdVideoH264SequenceParameterSetVui;
        */
        vkType.setCategory("struct");
        addMember(data, vkType, "StdVideoH264SpsVuiFlags", "flags");
        addMember(data, vkType, "StdVideoH264AspectRatioIdc", "aspect_ratio_idc");
        addMember(data, vkType, "uint16_t", "sar_width");
        addMember(data, vkType, "uint16_t", "sar_height");
        addMember(data, vkType, "uint8_t", "video_format");
        addMember(data, vkType, "uint8_t", "colour_primaries");
        addMember(data, vkType, "uint8_t", "transfer_characteristics");
        addMember(data, vkType, "uint8_t", "matrix_coefficients");
        addMember(data, vkType, "uint32_t", "num_units_in_tick");
        addMember(data, vkType, "uint32_t", "time_scale");
        addMember(data, vkType, "uint8_t", "max_num_reorder_frames");
        addMember(data, vkType, "uint8_t", "max_dec_frame_buffering");
        addMember(data, vkType, "uint8_t", "chroma_sample_loc_type_top_field");
        addMember(data, vkType, "uint8_t", "chroma_sample_loc_type_bottom_field");
        addMember(data, vkType, "uint32_t", "reserved1");
        addMember(data, vkType, "StdVideoH264HrdParameters", "pHrdParameters", true, true);

        vkType = getType(data, "StdVideoH264SpsFlags");
        vkType.sizeof = 4;

        vkType = getType(data, "StdVideoH264ChromaFormatIdc");
        vkType.sizeof = 4;

        vkType = getType(data, "StdVideoH264PocType");
        vkType.sizeof = 4;

        vkType = getType(data, "StdVideoH264SequenceParameterSet");
        /*
        typedef struct StdVideoH264SequenceParameterSet {
            StdVideoH264SpsFlags                          flags;
            StdVideoH264ProfileIdc                        profile_idc;
            StdVideoH264LevelIdc                          level_idc;
            StdVideoH264ChromaFormatIdc                   chroma_format_idc;
            uint8_t                                       seq_parameter_set_id;
            uint8_t                                       bit_depth_luma_minus8;
            uint8_t                                       bit_depth_chroma_minus8;
            uint8_t                                       log2_max_frame_num_minus4;
            StdVideoH264PocType                           pic_order_cnt_type;
            int32_t                                       offset_for_non_ref_pic;
            int32_t                                       offset_for_top_to_bottom_field;
            uint8_t                                       log2_max_pic_order_cnt_lsb_minus4;
            uint8_t                                       num_ref_frames_in_pic_order_cnt_cycle;
            uint8_t                                       max_num_ref_frames;
            uint8_t                                       reserved1;
            uint32_t                                      pic_width_in_mbs_minus1;
            uint32_t                                      pic_height_in_map_units_minus1;
            uint32_t                                      frame_crop_left_offset;
            uint32_t                                      frame_crop_right_offset;
            uint32_t                                      frame_crop_top_offset;
            uint32_t                                      frame_crop_bottom_offset;
            uint32_t                                      reserved2;
            const int32_t*                                pOffsetForRefFrame;
            const StdVideoH264ScalingLists*               pScalingLists;
            const StdVideoH264SequenceParameterSetVui*    pSequenceParameterSetVui;
        } StdVideoH264SequenceParameterSet;
        */
        vkType.setCategory("struct");
        addMember(data, vkType, "StdVideoH264SpsFlags", "flags");
        addMember(data, vkType, "StdVideoH264ProfileIdc", "profile_idc");
        addMember(data, vkType, "StdVideoH264LevelIdc", "level_idc");
        addMember(data, vkType, "StdVideoH264ChromaFormatIdc", "chroma_format_idc");
        addMember(data, vkType, "uint8_t", "seq_parameter_set_id");
        addMember(data, vkType, "uint8_t", "bit_depth_luma_minus8");
        addMember(data, vkType, "uint8_t", "bit_depth_chroma_minus8");
        addMember(data, vkType, "uint8_t", "log2_max_frame_num_minus4");
        addMember(data, vkType, "StdVideoH264PocType", "pic_order_cnt_type");
        addMember(data, vkType, "int32_t", "offset_for_non_ref_pic");
        addMember(data, vkType, "int32_t", "offset_for_top_to_bottom_field");
        addMember(data, vkType, "uint8_t", "log2_max_pic_order_cnt_lsb_minus4");
        addMember(data, vkType, "uint8_t", "num_ref_frames_in_pic_order_cnt_cycle");
        addMember(data, vkType, "uint8_t", "max_num_ref_frames");
        addMember(data, vkType, "uint8_t", "reserved1");
        addMember(data, vkType, "uint32_t", "pic_width_in_mbs_minus1");
        addMember(data, vkType, "uint32_t", "pic_height_in_map_units_minus1");
        addMember(data, vkType, "uint32_t", "frame_crop_left_offset");
        addMember(data, vkType, "uint32_t", "frame_crop_right_offset");
        addMember(data, vkType, "uint32_t", "frame_crop_top_offset");
        addMember(data, vkType, "uint32_t", "frame_crop_bottom_offset");
        addMember(data, vkType, "uint32_t", "reserved2");
        addMember(data, vkType, "int32_t", "pOffsetForRefFrame", true, true); // :TODO is this only 1 len
        addMember(data, vkType, "StdVideoH264ScalingLists", "pScalingLists", true, true);
        addMember(data, vkType, "StdVideoH264SequenceParameterSetVui", "pSequenceParameterSetVui", true, true);

        vkType = getType(data, "StdVideoH264PpsFlags");
        vkType.sizeof = 4;

        vkType = getType(data, "StdVideoH264WeightedBipredIdc");
        vkType.sizeof = 4;

        vkType = getType(data, "StdVideoH264PictureParameterSet");
        /*
        typedef struct StdVideoH264PictureParameterSet {
            StdVideoH264PpsFlags               flags;
            uint8_t                            seq_parameter_set_id;
            uint8_t                            pic_parameter_set_id;
            uint8_t                            num_ref_idx_l0_default_active_minus1;
            uint8_t                            num_ref_idx_l1_default_active_minus1;
            StdVideoH264WeightedBipredIdc      weighted_bipred_idc;
            int8_t                             pic_init_qp_minus26;
            int8_t                             pic_init_qs_minus26;
            int8_t                             chroma_qp_index_offset;
            int8_t                             second_chroma_qp_index_offset;
            const StdVideoH264ScalingLists*    pScalingLists;
        } StdVideoH264PictureParameterSet;
        */
        vkType.setCategory("struct");
        addMember(data, vkType, "StdVideoH264PpsFlags", "flags");
        addMember(data, vkType, "uint8_t", "seq_parameter_set_id");
        addMember(data, vkType, "uint8_t", "pic_parameter_set_id");
        addMember(data, vkType, "uint8_t", "num_ref_idx_l0_default_active_minus1");
        addMember(data, vkType, "uint8_t", "num_ref_idx_l1_default_active_minus1");
        addMember(data, vkType, "StdVideoH264WeightedBipredIdc", "weighted_bipred_idc");
        addMember(data, vkType, "int8_t", "pic_init_qp_minus26");
        addMember(data, vkType, "int8_t", "pic_init_qs_minus26");
        addMember(data, vkType, "int8_t", "chroma_qp_index_offset");
        addMember(data, vkType, "int8_t", "second_chroma_qp_index_offset");
        addMember(data, vkType, "StdVideoH264ScalingLists", "pScalingLists", true, true);

        vkType = getType(data, "StdVideoDecodeH265PictureInfoFlags");
        vkType.sizeof = 4;

        vkType = getType(data, "StdVideoDecodeH265PictureInfo");
        /*
        typedef struct StdVideoDecodeH265PictureInfo {
            StdVideoDecodeH265PictureInfoFlags    flags;
            uint8_t                               sps_video_parameter_set_id;
            uint8_t                               pps_seq_parameter_set_id;
            uint8_t                               pps_pic_parameter_set_id;
            uint8_t                               NumDeltaPocsOfRefRpsIdx;
            int32_t                               PicOrderCntVal;
            uint16_t                              NumBitsForSTRefPicSetInSlice;
            uint16_t                              reserved;
            uint8_t                               RefPicSetStCurrBefore[STD_VIDEO_DECODE_H265_REF_PIC_SET_LIST_SIZE];
            uint8_t                               RefPicSetStCurrAfter[STD_VIDEO_DECODE_H265_REF_PIC_SET_LIST_SIZE];
            uint8_t                               RefPicSetLtCurr[STD_VIDEO_DECODE_H265_REF_PIC_SET_LIST_SIZE];
        } StdVideoDecodeH265PictureInfo;
        */
        vkType.setCategory("struct");
        addMember(data, vkType, "StdVideoDecodeH265PictureInfoFlags", "flags");
        addMember(data, vkType, "uint8_t", "sps_video_parameter_set_id");
        addMember(data, vkType, "uint8_t", "pps_seq_parameter_set_id");
        addMember(data, vkType, "uint8_t", "pps_pic_parameter_set_id");
        addMember(data, vkType, "uint8_t", "NumDeltaPocsOfRefRpsIdx");
        addMember(data, vkType, "int32_t", "PicOrderCntVal");
        addMember(data, vkType, "uint16_t", "NumBitsForSTRefPicSetInSlice");
        addMember(data, vkType, "uint16_t", "reserved");
        addMember(data, vkType, "uint8_t", "RefPicSetStCurrBefore").arrayLen = 8;
        addMember(data, vkType, "uint8_t", "RefPicSetStCurrAfter").arrayLen = 8;
        addMember(data, vkType, "uint8_t", "RefPicSetLtCurr").arrayLen = 8;

        vkType = getType(data, "StdVideoDecodeH265ReferenceInfoFlags");
        vkType.sizeof = 4;

        vkType = getType(data, "StdVideoDecodeH265ReferenceInfo");
        /*
        typedef struct StdVideoDecodeH265ReferenceInfo {
            StdVideoDecodeH265ReferenceInfoFlags    flags;
            int32_t                                 PicOrderCntVal;
        } StdVideoDecodeH265ReferenceInfo;
        */
        vkType.setCategory("struct");
        addMember(data, vkType, "StdVideoDecodeH265ReferenceInfoFlags", "flags");
        addMember(data, vkType, "int32_t", "PicOrderCntVal");

        vkType = getType(data, "StdVideoEncodeH264PictureInfoFlags");
        vkType.sizeof = 4;

        vkType = getType(data, "StdVideoH264PictureType");
        vkType.sizeof = 4;

        vkType = getType(data, "StdVideoEncodeH264ReferenceListsInfoFlags");
        vkType.sizeof = 4;

        vkType = getType(data, "StdVideoH264ModificationOfPicNumsIdc");
        vkType.sizeof = 4;

        vkType = getType(data, "StdVideoEncodeH264RefListModEntry");
        /*
        typedef struct StdVideoEncodeH264RefListModEntry {
            StdVideoH264ModificationOfPicNumsIdc    modification_of_pic_nums_idc;
            uint16_t                                abs_diff_pic_num_minus1;
            uint16_t                                long_term_pic_num;
        } StdVideoEncodeH264RefListModEntry;
        */
        vkType.setCategory("struct");
        addMember(data, vkType, "StdVideoH264ModificationOfPicNumsIdc", "modification_of_pic_nums_idc");
        addMember(data, vkType, "uint16_t", "abs_diff_pic_num_minus1");
        addMember(data, vkType, "uint16_t", "long_term_pic_num");

        vkType = getType(data, "StdVideoH264MemMgmtControlOp");
        vkType.sizeof = 4;

        vkType = getType(data, "StdVideoEncodeH264RefPicMarkingEntry");
        /*
        typedef struct StdVideoEncodeH264RefPicMarkingEntry {
            StdVideoH264MemMgmtControlOp    memory_management_control_operation;
            uint16_t                        difference_of_pic_nums_minus1;
            uint16_t                        long_term_pic_num;
            uint16_t                        long_term_frame_idx;
            uint16_t                        max_long_term_frame_idx_plus1;
        } StdVideoEncodeH264RefPicMarkingEntry;
        */
        vkType.setCategory("struct");
        addMember(data, vkType, "StdVideoH264MemMgmtControlOp", "memory_management_control_operation");
        addMember(data, vkType, "uint16_t", "difference_of_pic_nums_minus1");
        addMember(data, vkType, "uint16_t", "long_term_pic_num");
        addMember(data, vkType, "uint16_t", "long_term_frame_idx");
        addMember(data, vkType, "uint16_t", "max_long_term_frame_idx_plus1");

        vkType = getType(data, "StdVideoEncodeH264ReferenceListsInfo");
        /*
        typedef struct StdVideoEncodeH264ReferenceListsInfo {
            StdVideoEncodeH264ReferenceListsInfoFlags      flags;
            uint8_t                                        num_ref_idx_l0_active_minus1;
            uint8_t                                        num_ref_idx_l1_active_minus1;
            uint8_t                                        RefPicList0[STD_VIDEO_H264_MAX_NUM_LIST_REF];
            uint8_t                                        RefPicList1[STD_VIDEO_H264_MAX_NUM_LIST_REF];
            uint8_t                                        refList0ModOpCount;
            uint8_t                                        refList1ModOpCount;
            uint8_t                                        refPicMarkingOpCount;
            uint8_t                                        reserved1[7];
            const StdVideoEncodeH264RefListModEntry*       pRefList0ModOperations;
            const StdVideoEncodeH264RefListModEntry*       pRefList1ModOperations;
            const StdVideoEncodeH264RefPicMarkingEntry*    pRefPicMarkingOperations;
        } StdVideoEncodeH264ReferenceListsInfo;
        */
        vkType.setCategory("struct");
        addMember(data, vkType, "StdVideoEncodeH264ReferenceListsInfoFlags", "flags");
        addMember(data, vkType, "uint8_t", "num_ref_idx_l0_active_minus1");
        addMember(data, vkType, "uint8_t", "num_ref_idx_l1_active_minus1");
        addMember(data, vkType, "uint8_t", "RefPicList0").arrayLen = 32;
        addMember(data, vkType, "uint8_t", "RefPicList1").arrayLen = 32;
        addMember(data, vkType, "uint8_t", "refList0ModOpCount");
        addMember(data, vkType, "uint8_t", "refList1ModOpCount");
        addMember(data, vkType, "uint8_t", "refPicMarkingOpCount");
        addMember(data, vkType, "uint8_t", "reserved1").arrayLen = 7;
        addMember(data, vkType, "StdVideoEncodeH264RefListModEntry", "pRefList0ModOperations", true, true);
        addMember(data, vkType, "StdVideoEncodeH264RefListModEntry", "pRefList1ModOperations", true, true);
        addMember(data, vkType, "StdVideoEncodeH264RefPicMarkingEntry", "pRefPicMarkingOperations", true, true);

        vkType = getType(data, "StdVideoEncodeH264PictureInfo");
        /*
        typedef struct StdVideoEncodeH264PictureInfo {
            StdVideoEncodeH264PictureInfoFlags             flags;
            uint8_t                                        seq_parameter_set_id;
            uint8_t                                        pic_parameter_set_id;
            uint16_t                                       idr_pic_id;
            StdVideoH264PictureType                        primary_pic_type;
            uint32_t                                       frame_num;
            int32_t                                        PicOrderCnt;
            uint8_t                                        temporal_id;
            uint8_t                                        reserved1[3];
            const StdVideoEncodeH264ReferenceListsInfo*    pRefLists;
        } StdVideoEncodeH264PictureInfo;
        */
        vkType.setCategory("struct");
        addMember(data, vkType, "StdVideoEncodeH264PictureInfoFlags", "flags");
        addMember(data, vkType, "uint8_t", "seq_parameter_set_id");
        addMember(data, vkType, "uint8_t", "pic_parameter_set_id");
        addMember(data, vkType, "uint16_t", "idr_pic_id");
        addMember(data, vkType, "StdVideoH264PictureType", "primary_pic_type");
        addMember(data, vkType, "uint32_t", "frame_num");
        addMember(data, vkType, "int32_t", "PicOrderCnt");
        addMember(data, vkType, "uint8_t", "temporal_id");
        addMember(data, vkType, "uint8_t", "reserved1").arrayLen = 3;
        addMember(data, vkType, "StdVideoEncodeH264ReferenceListsInfo", "pRefLists", true, true);

        vkType = getType(data, "StdVideoEncodeH264ReferenceInfoFlags");
        vkType.sizeof = 4;

        vkType = getType(data, "StdVideoEncodeH264ReferenceInfo");
        /*
        typedef struct StdVideoEncodeH264ReferenceInfo {
            StdVideoEncodeH264ReferenceInfoFlags    flags;
            StdVideoH264PictureType                 primary_pic_type;
            uint32_t                                FrameNum;
            int32_t                                 PicOrderCnt;
            uint16_t                                long_term_pic_num;
            uint16_t                                long_term_frame_idx;
            uint8_t                                 temporal_id;
        } StdVideoEncodeH264ReferenceInfo;
        */
        vkType.setCategory("struct");
        addMember(data, vkType, "StdVideoEncodeH264ReferenceInfoFlags", "flags");
        addMember(data, vkType, "StdVideoH264PictureType", "primary_pic_type");
        addMember(data, vkType, "uint32_t", "FrameNum");
        addMember(data, vkType, "int32_t", "PicOrderCnt");
        addMember(data, vkType, "uint16_t", "long_term_pic_num");
        addMember(data, vkType, "uint16_t", "long_term_frame_idx");
        addMember(data, vkType, "uint8_t", "temporal_id");

        vkType = getType(data, "StdVideoEncodeH265ReferenceInfoFlags");
        vkType.sizeof = 4;

        vkType = getType(data, "StdVideoEncodeH265ReferenceInfo");
        /*
        typedef struct StdVideoEncodeH265ReferenceInfo {
            StdVideoEncodeH265ReferenceInfoFlags    flags;
            StdVideoH265PictureType                 pic_type;
            int32_t                                 PicOrderCntVal;
            uint8_t                                 TemporalId;
        } StdVideoEncodeH265ReferenceInfo;
        */
        vkType.setCategory("struct");
        addMember(data, vkType, "StdVideoEncodeH265ReferenceInfoFlags", "flags");
        addMember(data, vkType, "StdVideoH265PictureType", "pic_type");
        addMember(data, vkType, "int32_t", "PicOrderCntVal");
        addMember(data, vkType, "uint8_t", "TemporalId");

        updateAV1(data);
    }

    // vk_video/vulkan_video_codec_av1std_encode.h
    static void updateAV1(VkData data) {
        VkType vkType = getType(data, "StdVideoEncodeAV1PictureInfoFlags");
        vkType.sizeof = 4;

        vkType = getType(data, "StdVideoAV1Profile");
        vkType.sizeof = 4;

        vkType = getType(data, "StdVideoAV1Level");
        vkType.sizeof = 4;

        vkType = getType(data, "StdVideoAV1FrameType");
        vkType.sizeof = 4;

        vkType = getType(data, "StdVideoAV1InterpolationFilter");
        vkType.sizeof = 4;

        vkType = getType(data, "StdVideoAV1TxMode");
        vkType.sizeof = 4;

        vkType = getType(data, "StdVideoAV1QuantizationFlags");
        vkType.sizeof = 4;

        vkType = getType(data, "StdVideoAV1Quantization");
        /*
        typedef struct StdVideoAV1Quantization {
            StdVideoAV1QuantizationFlags    flags;
            uint8_t                         base_q_idx;
            int8_t                          DeltaQYDc;
            int8_t                          DeltaQUDc;
            int8_t                          DeltaQUAc;
            int8_t                          DeltaQVDc;
            int8_t                          DeltaQVAc;
            uint8_t                         qm_y;
            uint8_t                         qm_u;
            uint8_t                         qm_v;
        } StdVideoAV1Quantization;
         */
        vkType.setCategory("struct");
        addMember(data, vkType, "StdVideoAV1QuantizationFlags", "flags");
        addMember(data, vkType, "uint8_t", "base_q_idx");
        addMember(data, vkType, "int8_t", "DeltaQYDc");
        addMember(data, vkType, "int8_t", "DeltaQUDc");
        addMember(data, vkType, "int8_t", "DeltaQUAc");
        addMember(data, vkType, "int8_t", "DeltaQVDc");
        addMember(data, vkType, "int8_t", "DeltaQVAc");
        addMember(data, vkType, "uint8_t", "qm_y");
        addMember(data, vkType, "uint8_t", "qm_u");
        addMember(data, vkType, "uint8_t", "qm_v");

        vkType = getType(data, "StdVideoAV1TileInfoFlags");
        vkType.sizeof = 4;

        vkType = getType(data, "StdVideoAV1TileInfo");
        /*
        typedef struct StdVideoAV1TileInfo {
            StdVideoAV1TileInfoFlags    flags;
            uint8_t                     TileCols;
            uint8_t                     TileRows;
            uint16_t                    context_update_tile_id;
            uint8_t                     tile_size_bytes_minus_1;
            uint8_t                     reserved1[7];
            const uint16_t*             pMiColStarts;
            const uint16_t*             pMiRowStarts;
            const uint16_t*             pWidthInSbsMinus1;
            const uint16_t*             pHeightInSbsMinus1;
        } StdVideoAV1TileInfo;
         */
        vkType.setCategory("struct");
        addMember(data, vkType, "StdVideoAV1TileInfoFlags", "flags");
        addMember(data, vkType, "uint8_t", "TileCols");
        addMember(data, vkType, "uint8_t", "TileRows");
        addMember(data, vkType, "uint16_t", "context_update_tile_id");
        addMember(data, vkType, "uint8_t", "tile_size_bytes_minus_1");
        addMember(data, vkType, "uint8_t", "reserved1").arrayLen = 7;
        addMember(data, vkType, "uint16_t", "pMiColStarts", true, true);
        addMember(data, vkType, "uint16_t", "pMiRowStarts", true, true);
        addMember(data, vkType, "uint16_t", "pWidthInSbsMinus1", true, true);
        addMember(data, vkType, "uint16_t", "pHeightInSbsMinus1", true, true);

        vkType = getType(data, "StdVideoAV1LoopFilterFlags");
        vkType.sizeof = 4;

        vkType = getType(data, "StdVideoAV1LoopFilter");
        /*
        typedef struct StdVideoAV1LoopFilter {
            StdVideoAV1LoopFilterFlags    flags;
            uint8_t                       loop_filter_level[STD_VIDEO_AV1_MAX_LOOP_FILTER_STRENGTHS];
            uint8_t                       loop_filter_sharpness;
            uint8_t                       update_ref_delta;
            int8_t                        loop_filter_ref_deltas[STD_VIDEO_AV1_TOTAL_REFS_PER_FRAME];
            uint8_t                       update_mode_delta;
            int8_t                        loop_filter_mode_deltas[STD_VIDEO_AV1_LOOP_FILTER_ADJUSTMENTS];
        } StdVideoAV1LoopFilter;
         */
        vkType.setCategory("struct");
        addMember(data, vkType, "StdVideoAV1LoopFilterFlags", "flags");
        addMember(data, vkType, "uint8_t", "loop_filter_level").arrayLen = 4;
        addMember(data, vkType, "uint8_t", "loop_filter_sharpness");
        addMember(data, vkType, "uint8_t", "update_ref_delta");
        addMember(data, vkType, "int8_t", "loop_filter_ref_deltas").arrayLen = 8;
        addMember(data, vkType, "uint8_t", "update_mode_delta");
        addMember(data, vkType, "int8_t", "loop_filter_mode_deltas").arrayLen = 2;

        vkType = getType(data, "StdVideoAV1Segmentation");
        /*
        typedef struct StdVideoAV1Segmentation {
            uint8_t    FeatureEnabled[STD_VIDEO_AV1_MAX_SEGMENTS];
            int16_t    FeatureData[STD_VIDEO_AV1_MAX_SEGMENTS][STD_VIDEO_AV1_SEG_LVL_MAX];
        } StdVideoAV1Segmentation;
         */
        vkType.setCategory("struct");
        addMember(data, vkType, "uint8_t", "FeatureEnabled").arrayLen = 8;
        addMember(data, vkType, "int16_t", "FeatureData").arrayLen = 64; // 8x8

        vkType = getType(data, "StdVideoAV1CDEF");
        /*
        typedef struct StdVideoAV1CDEF {
            uint8_t    cdef_damping_minus_3;
            uint8_t    cdef_bits;
            uint8_t    cdef_y_pri_strength[STD_VIDEO_AV1_MAX_CDEF_FILTER_STRENGTHS];
            uint8_t    cdef_y_sec_strength[STD_VIDEO_AV1_MAX_CDEF_FILTER_STRENGTHS];
            uint8_t    cdef_uv_pri_strength[STD_VIDEO_AV1_MAX_CDEF_FILTER_STRENGTHS];
            uint8_t    cdef_uv_sec_strength[STD_VIDEO_AV1_MAX_CDEF_FILTER_STRENGTHS];
        } StdVideoAV1CDEF;
         */
        vkType.setCategory("struct");
        addMember(data, vkType, "uint8_t", "cdef_damping_minus_3");
        addMember(data, vkType, "uint8_t", "cdef_bits");
        addMember(data, vkType, "uint8_t", "cdef_y_pri_strength").arrayLen = 8;
        addMember(data, vkType, "uint8_t", "cdef_y_sec_strength").arrayLen = 8;
        addMember(data, vkType, "uint8_t", "cdef_uv_pri_strength").arrayLen = 8;
        addMember(data, vkType, "uint8_t", "cdef_uv_sec_strength").arrayLen = 8;

        vkType = getType(data, "StdVideoAV1FrameRestorationType");
        vkType.sizeof = 4;

        vkType = getType(data, "StdVideoAV1LoopRestoration");
        /*
        typedef struct StdVideoAV1LoopRestoration {
            StdVideoAV1FrameRestorationType    FrameRestorationType[STD_VIDEO_AV1_MAX_NUM_PLANES];
            uint16_t                           LoopRestorationSize[STD_VIDEO_AV1_MAX_NUM_PLANES];
        } StdVideoAV1LoopRestoration;
         */
        vkType.setCategory("struct");
        addMember(data, vkType, "StdVideoAV1FrameRestorationType", "FrameRestorationType").arrayLen = 3;
        addMember(data, vkType, "uint16_t", "LoopRestorationSize").arrayLen = 3;

        vkType = getType(data, "StdVideoAV1GlobalMotion");
        /*
        typedef struct StdVideoAV1GlobalMotion {
            uint8_t    GmType[STD_VIDEO_AV1_NUM_REF_FRAMES];
            int32_t    gm_params[STD_VIDEO_AV1_NUM_REF_FRAMES][STD_VIDEO_AV1_GLOBAL_MOTION_PARAMS];
        } StdVideoAV1GlobalMotion;
         */
        vkType.setCategory("struct");
        addMember(data, vkType, "uint8_t", "GmType").arrayLen = 8;
        addMember(data, vkType, "int32_t", "gm_params").arrayLen = 48; // 8x6

        vkType = getType(data, "StdVideoEncodeAV1ExtensionHeader");
        /*
        typedef struct StdVideoEncodeAV1ExtensionHeader {
            uint8_t    temporal_id;
            uint8_t    spatial_id;
        } StdVideoEncodeAV1ExtensionHeader;
         */
        vkType.setCategory("struct");
        addMember(data, vkType, "uint8_t", "temporal_id");
        addMember(data, vkType, "uint8_t", "spatial_id");

        vkType = getType(data, "StdVideoEncodeAV1PictureInfo");
        /*
        typedef struct StdVideoEncodeAV1PictureInfo {
            StdVideoEncodeAV1PictureInfoFlags          flags;
            StdVideoAV1FrameType                       frame_type;
            uint32_t                                   frame_presentation_time;
            uint32_t                                   current_frame_id;
            uint8_t                                    order_hint;
            uint8_t                                    primary_ref_frame;
            uint8_t                                    refresh_frame_flags;
            uint8_t                                    coded_denom;
            uint16_t                                   render_width_minus_1;
            uint16_t                                   render_height_minus_1;
            StdVideoAV1InterpolationFilter             interpolation_filter;
            StdVideoAV1TxMode                          TxMode;
            uint8_t                                    delta_q_res;
            uint8_t                                    delta_lf_res;
            uint8_t                                    ref_order_hint[STD_VIDEO_AV1_NUM_REF_FRAMES];
            int8_t                                     ref_frame_idx[STD_VIDEO_AV1_REFS_PER_FRAME];
            uint8_t                                    reserved1[3];
            uint32_t                                   delta_frame_id_minus_1[STD_VIDEO_AV1_REFS_PER_FRAME];
            const StdVideoAV1TileInfo*                 pTileInfo;
            const StdVideoAV1Quantization*             pQuantization;
            const StdVideoAV1Segmentation*             pSegmentation;
            const StdVideoAV1LoopFilter*               pLoopFilter;
            const StdVideoAV1CDEF*                     pCDEF;
            const StdVideoAV1LoopRestoration*          pLoopRestoration;
            const StdVideoAV1GlobalMotion*             pGlobalMotion;
            const StdVideoEncodeAV1ExtensionHeader*    pExtensionHeader;
            const uint32_t*                            pBufferRemovalTimes;
        } StdVideoEncodeAV1PictureInfo;
        */
        vkType.setCategory("struct");
        addMember(data, vkType, "StdVideoEncodeAV1PictureInfoFlags", "flags");
        addMember(data, vkType, "StdVideoAV1FrameType", "frame_type");
        addMember(data, vkType, "uint32_t", "frame_presentation_time");
        addMember(data, vkType, "uint32_t", "current_frame_id");
        addMember(data, vkType, "uint8_t", "order_hint");
        addMember(data, vkType, "uint8_t", "primary_ref_frame");
        addMember(data, vkType, "uint8_t", "refresh_frame_flags");
        addMember(data, vkType, "uint8_t", "coded_denom");
        addMember(data, vkType, "uint16_t", "render_width_minus_1");
        addMember(data, vkType, "uint16_t", "render_height_minus_1");
        addMember(data, vkType, "StdVideoAV1InterpolationFilter", "interpolation_filter");
        addMember(data, vkType, "StdVideoAV1TxMode", "TxMode");
        addMember(data, vkType, "uint8_t", "delta_q_res");
        addMember(data, vkType, "uint8_t", "delta_lf_res");
        addMember(data, vkType, "uint8_t", "ref_order_hint").arrayLen = 8;
        addMember(data, vkType, "int8_t", "ref_frame_idx").arrayLen = 7;
        addMember(data, vkType, "uint8_t", "reserved1").arrayLen = 3;
        addMember(data, vkType, "uint32_t", "delta_frame_id_minus_1").arrayLen = 7;
        addMember(data, vkType, "StdVideoAV1TileInfo", "pTileInfo", true, true);
        addMember(data, vkType, "StdVideoAV1Quantization", "pQuantization", true, true);
        addMember(data, vkType, "StdVideoAV1Segmentation", "pSegmentation", true, true);
        addMember(data, vkType, "StdVideoAV1LoopFilter", "pLoopFilter", true, true);
        addMember(data, vkType, "StdVideoAV1CDEF", "pCDEF", true, true);
        addMember(data, vkType, "StdVideoAV1LoopRestoration", "pLoopRestoration", true, true);
        addMember(data, vkType, "StdVideoAV1GlobalMotion", "pGlobalMotion", true, true);
        addMember(data, vkType, "StdVideoEncodeAV1ExtensionHeader", "pExtensionHeader", true, true);
        // if flags.buffer_removal_time_present_flag is set, then pBufferRemovalTimes is a pointer to an array of N number of unsigned integer values
        // specifying the elements of the buffer_removal_time array, as defined in section 6.8.2 of the AV1 Specification, where N is the number of
        // operating points specified for the active sequence header through VkVideoEncodeAV1SessionParametersCreateInfoKHR::stdOperatingPointCount;
        addMember(data, vkType, "uint32_t", "pBufferRemovalTimes", true, true).arrayLen = 1; // :TODO

        vkType = getType(data, "StdVideoDecodeAV1ReferenceInfoFlags");
        vkType.sizeof = 4;

        vkType = getType(data, "StdVideoDecodeAV1ReferenceInfo");
        /*
        typedef struct StdVideoDecodeAV1ReferenceInfo {
            StdVideoDecodeAV1ReferenceInfoFlags    flags;
            uint8_t                                frame_type;
            uint8_t                                RefFrameSignBias;
            uint8_t                                OrderHint;
            uint8_t                                SavedOrderHints[STD_VIDEO_AV1_NUM_REF_FRAMES];
        } StdVideoDecodeAV1ReferenceInfo;
         */
        vkType.setCategory("struct");
        addMember(data, vkType, "StdVideoDecodeAV1ReferenceInfoFlags", "flags");
        addMember(data, vkType, "uint8_t", "frame_type");
        addMember(data, vkType, "uint8_t", "RefFrameSignBias");
        addMember(data, vkType, "uint8_t", "OrderHint");
        addMember(data, vkType, "uint8_t", "SavedOrderHints").arrayLen = 8;

        vkType = getType(data, "StdVideoEncodeAV1ReferenceInfoFlags");
        vkType.sizeof = 4;

        vkType = getType(data, "StdVideoEncodeAV1ReferenceInfo");
        /*
        typedef struct StdVideoEncodeAV1ReferenceInfo {
            StdVideoEncodeAV1ReferenceInfoFlags        flags;
            uint32_t                                   RefFrameId;
            StdVideoAV1FrameType                       frame_type;
            uint8_t                                    OrderHint;
            uint8_t                                    reserved1[3];
            const StdVideoEncodeAV1ExtensionHeader*    pExtensionHeader;
        } StdVideoEncodeAV1ReferenceInfo;
         */
        vkType.setCategory("struct");
        addMember(data, vkType, "StdVideoEncodeAV1ReferenceInfoFlags", "flags");
        addMember(data, vkType, "uint32_t", "RefFrameId");
        addMember(data, vkType, "StdVideoAV1FrameType", "frame_type");
        addMember(data, vkType, "uint8_t", "OrderHint");
        addMember(data, vkType, "uint8_t", "reserved1").arrayLen = 3;
        addMember(data, vkType, "StdVideoEncodeAV1ExtensionHeader", "pExtensionHeader", true, true);

        vkType = getType(data, "StdVideoAV1SequenceHeaderFlags");
        vkType.sizeof = 4;

        vkType = getType(data, "StdVideoAV1ColorConfigFlags");
        vkType.sizeof = 4;

        vkType = getType(data, "StdVideoAV1ColorPrimaries");
        vkType.sizeof = 4;

        vkType = getType(data, "StdVideoAV1TransferCharacteristics");
        vkType.sizeof = 4;

        vkType = getType(data, "StdVideoAV1MatrixCoefficients");
        vkType.sizeof = 4;

        vkType = getType(data, "StdVideoAV1TimingInfoFlags");
        vkType.sizeof = 4;

        vkType = getType(data, "StdVideoAV1TimingInfo");
        /*
        typedef struct StdVideoAV1TimingInfo {
            StdVideoAV1TimingInfoFlags    flags;
            uint32_t                      num_units_in_display_tick;
            uint32_t                      time_scale;
            uint32_t                      num_ticks_per_picture_minus_1;
        } StdVideoAV1TimingInfo;
         */
        vkType.setCategory("struct");
        addMember(data, vkType, "StdVideoAV1TimingInfoFlags", "flags");
        addMember(data, vkType, "uint32_t", "num_units_in_display_tick");
        addMember(data, vkType, "uint32_t", "time_scale");
        addMember(data, vkType, "uint32_t", "num_ticks_per_picture_minus_1");

        vkType = getType(data, "StdVideoAV1ChromaSamplePosition");
        vkType.sizeof = 4;

        vkType = getType(data, "StdVideoAV1ColorConfig");
        /*
        typedef struct StdVideoAV1ColorConfig {
            StdVideoAV1ColorConfigFlags           flags;
            uint8_t                               BitDepth;
            uint8_t                               subsampling_x;
            uint8_t                               subsampling_y;
            uint8_t                               reserved1;
            StdVideoAV1ColorPrimaries             color_primaries;
            StdVideoAV1TransferCharacteristics    transfer_characteristics;
            StdVideoAV1MatrixCoefficients         matrix_coefficients;
            StdVideoAV1ChromaSamplePosition       chroma_sample_position;
        } StdVideoAV1ColorConfig;
         */
        vkType.setCategory("struct");
        addMember(data, vkType, "StdVideoAV1ColorConfigFlags", "flags");
        addMember(data, vkType, "uint8_t", "BitDepth");
        addMember(data, vkType, "uint8_t", "subsampling_x");
        addMember(data, vkType, "uint8_t", "subsampling_y");
        addMember(data, vkType, "uint8_t", "reserved1");
        addMember(data, vkType, "StdVideoAV1ColorPrimaries", "color_primaries");
        addMember(data, vkType, "StdVideoAV1TransferCharacteristics", "transfer_characteristics");
        addMember(data, vkType, "StdVideoAV1MatrixCoefficients", "matrix_coefficients");
        addMember(data, vkType, "StdVideoAV1ChromaSamplePosition", "chroma_sample_position");

        vkType = getType(data, "StdVideoAV1SequenceHeader");
        /*
        typedef struct StdVideoAV1SequenceHeader {
            StdVideoAV1SequenceHeaderFlags    flags;
            StdVideoAV1Profile                seq_profile;
            uint8_t                           frame_width_bits_minus_1;
            uint8_t                           frame_height_bits_minus_1;
            uint16_t                          max_frame_width_minus_1;
            uint16_t                          max_frame_height_minus_1;
            uint8_t                           delta_frame_id_length_minus_2;
            uint8_t                           additional_frame_id_length_minus_1;
            uint8_t                           order_hint_bits_minus_1;
            uint8_t                           seq_force_integer_mv;
            uint8_t                           seq_force_screen_content_tools;
            uint8_t                           reserved1[5];
            const StdVideoAV1ColorConfig*     pColorConfig;
            const StdVideoAV1TimingInfo*      pTimingInfo;
        } StdVideoAV1SequenceHeader;
         */
        vkType.setCategory("struct");
        addMember(data, vkType, "StdVideoAV1SequenceHeaderFlags", "flags");
        addMember(data, vkType, "StdVideoAV1Profile", "seq_profile");
        addMember(data, vkType, "uint8_t", "frame_width_bits_minus_1");
        addMember(data, vkType, "uint8_t", "frame_height_bits_minus_1");
        addMember(data, vkType, "uint16_t", "max_frame_width_minus_1");
        addMember(data, vkType, "uint16_t", "max_frame_height_minus_1");
        addMember(data, vkType, "uint8_t", "delta_frame_id_length_minus_2");
        addMember(data, vkType, "uint8_t", "additional_frame_id_length_minus_1");
        addMember(data, vkType, "uint8_t", "order_hint_bits_minus_1");
        addMember(data, vkType, "uint8_t", "seq_force_integer_mv");
        addMember(data, vkType, "uint8_t", "seq_force_screen_content_tools");
        addMember(data, vkType, "uint8_t", "reserved1").arrayLen = 5;
        addMember(data, vkType, "StdVideoAV1ColorConfig", "pColorConfig", true, true);
        addMember(data, vkType, "StdVideoAV1TimingInfo", "pTimingInfo", true, true);

        vkType = getType(data, "StdVideoDecodeAV1PictureInfoFlags");
        vkType.sizeof = 4;

        vkType = getType(data, "StdVideoAV1InterpolationFilter");
        vkType.sizeof = 4;

        vkType = getType(data, "StdVideoAV1FilmGrainFlags");
        vkType.sizeof = 4;

        vkType = getType(data, "StdVideoAV1FilmGrain");
        /*
        typedef struct StdVideoAV1FilmGrain {
            StdVideoAV1FilmGrainFlags    flags;
            uint8_t                      grain_scaling_minus_8;
            uint8_t                      ar_coeff_lag;
            uint8_t                      ar_coeff_shift_minus_6;
            uint8_t                      grain_scale_shift;
            uint16_t                     grain_seed;
            uint8_t                      film_grain_params_ref_idx;
            uint8_t                      num_y_points;
            uint8_t                      point_y_value[STD_VIDEO_AV1_MAX_NUM_Y_POINTS];
            uint8_t                      point_y_scaling[STD_VIDEO_AV1_MAX_NUM_Y_POINTS];
            uint8_t                      num_cb_points;
            uint8_t                      point_cb_value[STD_VIDEO_AV1_MAX_NUM_CB_POINTS];
            uint8_t                      point_cb_scaling[STD_VIDEO_AV1_MAX_NUM_CB_POINTS];
            uint8_t                      num_cr_points;
            uint8_t                      point_cr_value[STD_VIDEO_AV1_MAX_NUM_CR_POINTS];
            uint8_t                      point_cr_scaling[STD_VIDEO_AV1_MAX_NUM_CR_POINTS];
            int8_t                       ar_coeffs_y_plus_128[STD_VIDEO_AV1_MAX_NUM_POS_LUMA];
            int8_t                       ar_coeffs_cb_plus_128[STD_VIDEO_AV1_MAX_NUM_POS_CHROMA];
            int8_t                       ar_coeffs_cr_plus_128[STD_VIDEO_AV1_MAX_NUM_POS_CHROMA];
            uint8_t                      cb_mult;
            uint8_t                      cb_luma_mult;
            uint16_t                     cb_offset;
            uint8_t                      cr_mult;
            uint8_t                      cr_luma_mult;
            uint16_t                     cr_offset;
        } StdVideoAV1FilmGrain;
         */
        vkType.setCategory("struct");
        addMember(data, vkType, "StdVideoAV1FilmGrainFlags", "flags");
        addMember(data, vkType, "uint8_t", "grain_scaling_minus_8");
        addMember(data, vkType, "uint8_t", "ar_coeff_lag");
        addMember(data, vkType, "uint8_t", "ar_coeff_shift_minus_6");
        addMember(data, vkType, "uint8_t", "grain_scale_shift");
        addMember(data, vkType, "uint16_t", "grain_seed");
        addMember(data, vkType, "uint8_t", "film_grain_params_ref_idx");
        addMember(data, vkType, "uint8_t", "num_y_points");
        addMember(data, vkType, "uint8_t", "point_y_value").arrayLen = 14;
        addMember(data, vkType, "uint8_t", "point_y_scaling").arrayLen = 14;
        addMember(data, vkType, "uint8_t", "num_cb_points");
        addMember(data, vkType, "uint8_t", "point_cb_value").arrayLen = 10;
        addMember(data, vkType, "uint8_t", "point_cb_scaling").arrayLen = 10;
        addMember(data, vkType, "uint8_t", "num_cr_points");
        addMember(data, vkType, "uint8_t", "point_cr_value").arrayLen = 10;
        addMember(data, vkType, "uint8_t", "point_cr_scaling").arrayLen = 10;
        addMember(data, vkType, "int8_t", "ar_coeffs_y_plus_128").arrayLen = 24;
        addMember(data, vkType, "int8_t", "ar_coeffs_cb_plus_128").arrayLen = 25;
        addMember(data, vkType, "int8_t", "ar_coeffs_cr_plus_128").arrayLen = 25;
        addMember(data, vkType, "uint8_t", "cb_mult");
        addMember(data, vkType, "uint8_t", "cb_luma_mult");
        addMember(data, vkType, "uint16_t", "cb_offset");
        addMember(data, vkType, "uint8_t", "cr_mult");
        addMember(data, vkType, "uint8_t", "cr_luma_mult");
        addMember(data, vkType, "uint16_t", "cr_offset");

        vkType = getType(data, "StdVideoDecodeAV1PictureInfo");
        /*
        typedef struct StdVideoDecodeAV1PictureInfo {
            StdVideoDecodeAV1PictureInfoFlags    flags;
            StdVideoAV1FrameType                 frame_type;
            uint32_t                             current_frame_id;
            uint8_t                              OrderHint;
            uint8_t                              primary_ref_frame;
            uint8_t                              refresh_frame_flags;
            uint8_t                              reserved1;
            StdVideoAV1InterpolationFilter       interpolation_filter;
            StdVideoAV1TxMode                    TxMode;
            uint8_t                              delta_q_res;
            uint8_t                              delta_lf_res;
            uint8_t                              SkipModeFrame[STD_VIDEO_AV1_SKIP_MODE_FRAMES];
            uint8_t                              coded_denom;
            uint8_t                              reserved2[3];
            uint8_t                              OrderHints[STD_VIDEO_AV1_NUM_REF_FRAMES];
            uint32_t                             expectedFrameId[STD_VIDEO_AV1_NUM_REF_FRAMES];
            const StdVideoAV1TileInfo*           pTileInfo;
            const StdVideoAV1Quantization*       pQuantization;
            const StdVideoAV1Segmentation*       pSegmentation;
            const StdVideoAV1LoopFilter*         pLoopFilter;
            const StdVideoAV1CDEF*               pCDEF;
            const StdVideoAV1LoopRestoration*    pLoopRestoration;
            const StdVideoAV1GlobalMotion*       pGlobalMotion;
            const StdVideoAV1FilmGrain*          pFilmGrain;
        } StdVideoDecodeAV1PictureInfo;
         */
        vkType.setCategory("struct");
        addMember(data, vkType, "StdVideoDecodeAV1PictureInfoFlags", "flags");
        addMember(data, vkType, "StdVideoAV1FrameType", "frame_type");
        addMember(data, vkType, "uint32_t", "current_frame_id");
        addMember(data, vkType, "uint8_t", "OrderHint");
        addMember(data, vkType, "uint8_t", "primary_ref_frame");
        addMember(data, vkType, "uint8_t", "refresh_frame_flags");
        addMember(data, vkType, "uint8_t", "reserved1");
        addMember(data, vkType, "StdVideoAV1InterpolationFilter", "interpolation_filter");
        addMember(data, vkType, "StdVideoAV1TxMode", "TxMode");
        addMember(data, vkType, "uint8_t", "delta_q_res");
        addMember(data, vkType, "uint8_t", "delta_lf_res");
        addMember(data, vkType, "uint8_t", "SkipModeFrame").arrayLen = 2;
        addMember(data, vkType, "uint8_t", "coded_denom");
        addMember(data, vkType, "uint8_t", "reserved2").arrayLen = 3;
        addMember(data, vkType, "uint8_t", "OrderHints").arrayLen = 8;
        addMember(data, vkType, "uint32_t", "expectedFrameId").arrayLen = 8;
        addMember(data, vkType, "StdVideoAV1TileInfo", "pTileInfo", true, true);
        addMember(data, vkType, "StdVideoAV1Quantization", "pQuantization", true, true);
        addMember(data, vkType, "StdVideoAV1Segmentation", "pSegmentation", true, true);
        addMember(data, vkType, "StdVideoAV1LoopFilter", "pLoopFilter", true, true);
        addMember(data, vkType, "StdVideoAV1CDEF", "pCDEF", true, true);
        addMember(data, vkType, "StdVideoAV1LoopRestoration", "pLoopRestoration", true, true);
        addMember(data, vkType, "StdVideoAV1GlobalMotion", "pGlobalMotion", true, true);
        addMember(data, vkType, "StdVideoAV1FilmGrain", "pFilmGrain", true, true);

        vkType = getType(data, "StdVideoEncodeAV1DecoderModelInfo");
        /*
        typedef struct StdVideoEncodeAV1DecoderModelInfo {
            uint8_t     buffer_delay_length_minus_1;
            uint8_t     buffer_removal_time_length_minus_1;
            uint8_t     frame_presentation_time_length_minus_1;
            uint8_t     reserved1;
            uint32_t    num_units_in_decoding_tick;
        } StdVideoEncodeAV1DecoderModelInfo;
         */
        vkType.setCategory("struct");
        addMember(data, vkType, "uint8_t", "buffer_delay_length_minus_1");
        addMember(data, vkType, "uint8_t", "buffer_removal_time_length_minus_1");
        addMember(data, vkType, "uint8_t", "frame_presentation_time_length_minus_1");
        addMember(data, vkType, "uint8_t", "reserved1");
        addMember(data, vkType, "uint32_t", "num_units_in_decoding_tick");

        vkType = getType(data, "StdVideoEncodeAV1OperatingPointInfoFlags");
        vkType.sizeof = 4;

        vkType = getType(data, "StdVideoEncodeAV1OperatingPointInfo");
        /*
        typedef struct StdVideoEncodeAV1OperatingPointInfo {
            StdVideoEncodeAV1OperatingPointInfoFlags    flags;
            uint16_t                                    operating_point_idc;
            uint8_t                                     seq_level_idx;
            uint8_t                                     seq_tier;
            uint32_t                                    decoder_buffer_delay;
            uint32_t                                    encoder_buffer_delay;
            uint8_t                                     initial_display_delay_minus_1;
        } StdVideoEncodeAV1OperatingPointInfo;
         */
        vkType.setCategory("struct");
        addMember(data, vkType, "StdVideoEncodeAV1OperatingPointInfoFlags", "flags");
        addMember(data, vkType, "uint16_t", "operating_point_idc");
        addMember(data, vkType, "uint8_t", "seq_level_idx");
        addMember(data, vkType, "uint8_t", "seq_tier");
        addMember(data, vkType, "uint32_t", "decoder_buffer_delay");
        addMember(data, vkType, "uint32_t", "encoder_buffer_delay");
        addMember(data, vkType, "uint8_t", "initial_display_delay_minus_1");
    }

    static void updateType(VkData data, String name, String type, String category, int sizeof) {
        VkType vkType = data.types.get(name);
        if (vkType == null) {
            vkType = new VkType(name, type, category, sizeof);
            data.types.put(name, vkType);
            data.orderedTypes.add(vkType);
        } else {
            vkType.setType(type);
            vkType.setCategory(category);
            vkType.sizeof = sizeof;
        }
    }
    static void setTypeEmulatedSize(VkData data, VkType vkType) throws Exception {
        if (vkType.sizeof == 0) {
            if (vkType.getCategory().equals("struct")) {
                // Boxedwine runs wine in an emulated 32-bit linux environment
                // This means 8 byte members are aligned to 4 byte boundaries
                int offset = 0;
                vkType.alignment = 0;
                for (VkParam member : vkType.members) {
                    if (!member.postProcessed) {
                        postParseParam(data, member);
                    }
                    if (member.paramType == null) {
                        int ii=0;
                    }
                    if (member.paramType.sizeof == 0) {
                        setTypeEmulatedSize(data, member.paramType);
                    }
                    if (member.paramType.sizeof == 0 && !member.paramType.name.equals("void")) {
                        throw new Exception("Could not determine sizeof " + member.paramType.name);
                    }
                    int size;
                    int align = 1;
                    if (member.isPointer) {
                        size = 4;
                        align = 4;
                    } else if (member.arrayLen == 0) {
                        size = member.paramType.sizeof;
                        align = Math.min(member.paramType.alignment, 4);
                    } else {
                        size = member.paramType.sizeof * member.arrayLen;
                        align = Math.min(member.paramType.alignment, 4);
                    }
                    if (size == 0) {
                        size = member.sizeof;
                        align = Math.min(size, 4);
                    }
                    if (align == 0 && member.paramType.members.size() == 0) {
                        // :TODO: why was member.paramType.alignment 0
                        align = Math.min(member.paramType.sizeof, 4);
                    }

                    vkType.alignment = Math.max(vkType.alignment, align);
                    if (offset > 0 && (offset % align) != 0) {
                        int adjust = align - ((offset % align));
                        offset += adjust;
                    }
                    member.offset = offset;
                    offset += size;
                    vkType.sizeof = offset;
                }
                if (vkType.alignment == 0) {
                    int ii=0;
                }
                if ((vkType.sizeof % vkType.alignment) != 0) {
                    int adjust = vkType.alignment - ((vkType.sizeof % vkType.alignment));
                    vkType.sizeof += adjust;
                }
            } else if (vkType.getCategory().equals("union")) {
                for (VkParam member : vkType.members) {
                    if (member.paramType.sizeof == 0) {
                        setTypeEmulatedSize(data, member.paramType);
                    }
                    if (member.paramType.sizeof == 0 && !member.paramType.name.equals("void")) {
                        throw new Exception("Could not determine sizeof " + member.paramType.name);
                    }
                    if (member.isPointer) {
                        member.sizeof = 4;
                    } else {
                        member.sizeof = member.paramType.sizeof;
                    }
                    if (member.arrayLen != 0) {
                        member.sizeof = member.sizeof * member.arrayLen;
                    }
                    vkType.sizeof = Math.max(vkType.sizeof, member.sizeof);
                    vkType.alignment = Math.max(vkType.alignment, member.paramType.alignment);
                }
            } else if (vkType.getType().equals("VK_DEFINE_NON_DISPATCHABLE_HANDLE")) {
                vkType.sizeof = 8;
            } else if (vkType.getType().equals("VK_DEFINE_HANDLE")) {
                vkType.sizeof = 4;
            } else if (vkType.getCategory().equals("enum")) {
                vkType.sizeof = 4;
            } else if (vkType.getType().equals("VkFlags")) {
                vkType.sizeof = 4;
            } else if (vkType.getType().equals("VkFlags64")) {
                vkType.sizeof = 8;
            } else if (vkType.getType().equals("uint32_t")) {
                vkType.sizeof = 4;
            } else if (vkType.getType().equals("uint64_t")) {
                vkType.sizeof = 8;
            } else if (vkType.getCategory().equals("funcpointer")) {
                vkType.sizeof = 4;
            } else if ( !vkType.name.equals("void")) {
                throw new Exception("Could not determine sizeof " + vkType.name);
            }
            if (vkType.alignment == 0) {
                vkType.alignment = Math.min(vkType.sizeof, 4);
            }
        }
    }

    static void setTypeEmulatedSizes(VkData data) throws Exception {
        int index = 0;
        for (VkType vkType : data.orderedTypes) {
            setTypeEmulatedSize(data, vkType);
            index++;
        }
    }

    static void addDefaultTypes(VkData data) {
        updateType(data, "char", "char", "platform", 1);
        updateType(data, "float", "float", "platform", 4);
        updateType(data, "double", "double", "platform", 8);
        updateType(data, "int8_t", "int8_t", "platform", 1);
        updateType(data, "uint8_t", "uint8_t", "platform", 1);
        updateType(data, "int16_t", "int16_t", "platform", 2);
        updateType(data, "uint16_t", "uint16_t", "platform", 2);
        updateType(data, "int32_t", "int32_t", "platform", 4);
        updateType(data, "int", "int", "platform", 4);
        updateType(data, "uint32_t", "uint32_t", "platform", 4);
        updateType(data, "int64_t", "int64_t", "platform", 8);
        updateType(data, "uint64_t", "uint64_t", "platform", 8);
        updateType(data, "size_t", "size_t", "platform", 4);

        // X11/Xlib.h
        updateType(data, "Display", null, null, 4);
        updateType(data, "VisualID", null, null, 4);
        updateType(data, "Window", null, null, 4);
    }

    static Set<String> getUnsupportedExtensions() {
        HashSet<String> unsupportedExtensions = new HashSet<>();

        // this list was copied from make_vulkan in wine

        // Instance extensions
        unsupportedExtensions.add("VK_EXT_headless_surface"); // Needs WSI work.
        unsupportedExtensions.add("VK_KHR_display"); // Needs WSI work.
        unsupportedExtensions.add("VK_KHR_surface_protected_capabilities");
        unsupportedExtensions.add("VK_LUNARG_direct_driver_loading"); // Implemented in the Vulkan loader

        // Device extensions
        unsupportedExtensions.add("VK_AMD_display_native_hdr");
        unsupportedExtensions.add("VK_EXT_full_screen_exclusive");
        unsupportedExtensions.add("VK_GOOGLE_display_timing");
        unsupportedExtensions.add("VK_KHR_external_fence_win32");
        unsupportedExtensions.add("VK_KHR_external_semaphore_win32");
        // Relates to external_semaphore and needs type conversions in bitflags.
        unsupportedExtensions.add("VK_KHR_maintenance7"); // Causes infinity recursion in struct convert code
        unsupportedExtensions.add("VK_KHR_shared_presentable_image"); // Needs WSI work.
        unsupportedExtensions.add("VK_KHR_win32_keyed_mutex");
        unsupportedExtensions.add("VK_NV_external_memory_rdma"); // Needs shared resources work.

        // Extensions for other platforms
        unsupportedExtensions.add("VK_EXT_external_memory_dma_buf");
        unsupportedExtensions.add("VK_EXT_image_drm_format_modifier");
        unsupportedExtensions.add("VK_EXT_metal_objects");
        unsupportedExtensions.add("VK_EXT_physical_device_drm");
        unsupportedExtensions.add("VK_GOOGLE_surfaceless_query");
        unsupportedExtensions.add("VK_KHR_external_fence_fd");
        unsupportedExtensions.add("VK_KHR_external_memory_fd");
        unsupportedExtensions.add("VK_KHR_external_semaphore_fd");
        unsupportedExtensions.add("VK_SEC_amigo_profiling"); // Angle specific.

        // Extensions which require callback handling
        unsupportedExtensions.add("VK_EXT_device_memory_report");

        // Deprecated extensions
        unsupportedExtensions.add("VK_NV_external_memory_capabilities");
        unsupportedExtensions.add("VK_NV_external_memory_win32");
        return unsupportedExtensions;
    }

    static private void removeUnsupportedExtensions(VkData data) {
        Set<String> unsupportedExtensions = getUnsupportedExtensions();
        for (VkExtension extension : data.extensions) {
            if (extension.platform != null && !extension.platform.equals("xlib")) {
                removeExtension(data, extension);
                continue;
            }
            if (extension.supported != null) {
                Vector<String> supported = new Vector<String>(Arrays.asList(extension.supported.split(",")));
                if (!supported.contains("vulkan")) {
                    removeExtension(data, extension);
                    continue;
                }
            }
            if (unsupportedExtensions.contains(extension.name)) {
                removeExtension(data, extension);
                continue;
            }
            for (VkExtension.VkExtensionRequire require : extension.require) {
                if (require.api != null) {
                    Vector<String> apis = new Vector<String>(Arrays.asList(require.api.split(",")));
                    if (!apis.contains("vulkan")) {
                        removeExtensionRequire(data, require);
                        continue;
                    }
                }
            }
        }
    }

    static private void removeUnsupportedFeatures(VkData data) {
        for (VkFeature feature : data.features) {
            if (feature.api != null) {
                Vector<String> apis = new Vector<String>(Arrays.asList(feature.api.split(",")));
                if (!apis.contains("vulkan")) {
                    removeFeature(data, feature);
                }
            }
        }
    }

    static private void removeFunction(VkData data, String name) {
        for (VkFunction vkFunction : data.functions) {
            if (vkFunction.name.equals(name)) {
                data.functions.remove(vkFunction);
                return;
            }
        }
    }
    static private void removeType(VkData data, String name) {
        for (VkType vkType : data.types.values()) {
            if (vkType.name.equals(name)) {
                data.types.remove(vkType.name);
                data.orderedTypes.remove(vkType);
                return;
            }
        }
    }
    static private void removeFeature(VkData data, VkFeature feature) {
        System.out.println("Removing extension: "+feature.name+" "+(feature.api!=null?feature.api+" ":""));
        for (String function : feature.functions) {
            VkFunction vkFunction = data.getFunctionByName(function);
            if (vkFunction != null) {
                data.functions.remove(vkFunction);
            }
        }
        for (String name : feature.types) {
            VkType vkType = data.types.get(name);
            if (vkType != null) {
                data.types.remove(name);
                data.orderedTypes.remove(vkType);
            }
        }
        for (String name : feature.extendsStructures) {
            data.ignoreStructTypes.add(name);
        }
    }
    static private void removeExtension(VkData data, VkExtension extension) {
        System.out.println("Removing extension: "+extension.name+" "+extension.supported+" "+(extension.platform!=null?extension.platform+" ":"")+(getUnsupportedExtensions().contains(extension.name)?"explicityly removed":""));
        for (VkExtension.VkExtensionRequire vkRequire : extension.require) {
            removeExtensionRequire(data, vkRequire);
        }
    }
    static private void removeExtensionRequire(VkData data, VkExtension.VkExtensionRequire vkRequire) {
        for (VkType vkType : vkRequire.types) {
            data.types.remove(vkType.name);
            data.orderedTypes.remove(vkType);
        }
        for (VkFunction vkFunction : vkRequire.functions) {
            data.functions.remove(vkFunction);
        }
        for (String structType : vkRequire.extendsStructures) {
            data.ignoreStructTypes.add(structType);
        }
    }
    static private void postParseParam(VkData data, VkParam param) {
        if (param.full == null) {
            return;
        }
        param.isConst = param.full.contains("const ");
        if (param.full.contains("[")) {
            int pos = param.full.indexOf('[');
            int pos2 = param.full.indexOf(']');
            String len = param.full.substring(pos + 1, pos2);
            try {
                param.arrayLen = Integer.parseInt(len);
            } catch (Exception e) {
                String constant = data.constants.get(len);
                if (constant != null) {
                    param.arrayLen = Integer.parseInt(constant);
                }
            }
        } else {
            param.isDoublePointer = param.full.chars().filter(ch -> ch == '*').count() == 2;
            if (param.isDoublePointer) {
                param.isDoublePointer = param.full.contains("* const*") || param.full.contains("* const *") || param.full.contains("**");
                if (!param.isDoublePointer) {
                    System.out.println("Denide Double Pointer Status: " + param.full);
                }
                if (param.name.equals("ppBuildRangeInfos")) {
                    param.secondArrayLen = "pInfos[i].geometryCount";
                } else if (param.name.equals("ppMaxPrimitiveCounts")) {
                    param.secondArrayLen = "pInfos[i].geometryCount";
                }
            }
            param.isPointer = param.full.contains("*");
        }
        param.postProcessed = true;
        for (VkParam member : param.paramType.members) {
            if (!member.postProcessed) {
                postParseParam(data, member);
            }
        }
    }
    static private void postParseParams(VkData data) {
        for (VkFunction fn : data.functions) {
            if (fn.name.equals("vkCmdDrawMultiIndexedEXT")) {
                fn.params.get(6).countParam = fn.params.get(1);
                fn.params.get(6).len = fn.params.get(1).name;
            }
            for (VkParam param : fn.params) {
                if (param.len != null && param.countParam == null) {
                    for (VkParam p : fn.params) {
                        if (p.name.equals(param.len)) {
                            param.countParam = p;
                            break;
                        }
                    }
                }
                postParseParam(data, param);
            }
        }
    }
    static private void postParseTypes(VkData data) {
        for (VkType vkType : data.orderedTypes) {
            for (VkParam vkParam : vkType.members) {
                if (vkParam.name.equals("pStdSliceHeader") && vkType.name.equals("VkVideoEncodeH264NaluSliceInfoKHR")) {
                    // doc pStdSliceHeader is a pointer to a StdVideoEncodeH264SliceHeader structure specifying H.264 slice header parameters for the slice.
                    //
                    // I assume this means 1
                    vkParam.arrayLen = 1;
                }
                // this is a user defined type, so no marshalling needed
                if (vkParam.name.equals("pCheckpointMarker")) {
                    vkParam.isPointer = false;
                    vkParam.sizeof = 4;
                }
                if (vkParam.name.equals("pUserData")) {
                    vkParam.isPointer = false;
                    vkParam.sizeof = 4;
                }
            }
        }
    }
}
