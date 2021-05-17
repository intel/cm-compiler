
set(CM_HEADERS
  cm/cm_atomic.h
  cm/cm_common.h
  cm/cm_dataport.h
  cm/cm_gateway.h
  cm/cm.h
  cm/cm_internal.h
  cm/cm_library.h
  cm/cm_linear.h
  cm/cm_mask.h
  cm/cm_printf.h
  cm/cm_printfocl.h
  cm/cm_sampler.h
  cm/cm_send.h
  cm/cm_svm.h
  cm/cm_target.h
  cm/cmtl/global.h
  cm/cmtl.h
  cm/cmtl/hint.h
  cm/cmtl/mask.h
  cm/cmtl/math/clamp.h
  cm/cmtl/math/exp.h
  cm/cmtl/math/exp_impl.h
  cm/cmtl/math/float.h
  cm/cmtl/math/fpclassify.h
  cm/cmtl/math/frexp.h
  cm/cmtl/math/frexp_impl.h
  cm/cmtl/math.h
  cm/cmtl/math/log.h
  cm/cmtl/math/log_impl.h
  cm/cmtl/math/round.h
  cm/cmtl/math/utils.h
  cm/cmtl/numbers.h
  cm/cm_traits.h
  cm/cm_util.h
  cm/cm_vme.h
  cm/cm_vs2013.h
  cm/gen10_vme.h
  cm/gen11_vme.h
  cm/gen12_vme.h
  cm/gen7_5_vme.h
  cm/gen8_vme.h
  cm/gen9_vme.h

  opencl-c-common.h
  )

set(LLVM_LINK_COMPONENTS
  Support
  )

add_llvm_utility(ResourceEmbedder
  ResourceEmbedder.cpp
  )

set(OUTPUT_FILE "${CMAKE_CURRENT_BINARY_DIR}/EmbeddedHeaders.inc")
add_custom_command(OUTPUT ${OUTPUT_FILE}
  COMMAND
  ResourceEmbedder
  ${CM_HEADERS}
  -output=${OUTPUT_FILE}
  DEPENDS
  ${CM_HEADERS}
  ResourceEmbedder
  WORKING_DIRECTORY
  "${CMAKE_CURRENT_SOURCE_DIR}"
  )

add_custom_target(EmbedCmHeaders
  SOURCES ${CM_HEADERS}
  DEPENDS ${OUTPUT_FILE}
  )

add_library(CmEmbeddedHeaders INTERFACE)
target_include_directories(CmEmbeddedHeaders
  INTERFACE
  ${CMAKE_CURRENT_BINARY_DIR}
  )
add_dependencies(CmEmbeddedHeaders EmbedCmHeaders)
