set(LLVM_LINK_COMPONENTS
  Analysis
  MC
  ScalarOpts
  TransformUtils
  )

add_llvm_target(GenXCodeGen
  FunctionGroup.cpp
  KillAnalysis.cpp
  GenXAddressCommoning.cpp
  GenXAlignmentInfo.cpp
  GenXAnalysisDumper.cpp
  GenXArgIndirection.cpp
  GenXBaling.cpp
  GenXCategory.cpp
  GenXCFSimplification.cpp
  GenXConstants.cpp
  GenXCoalescing.cpp
  GenXDeadVectorRemoval.cpp
  GenXDepressurizer.cpp
  GenXExtractVectorizer.cpp
  GenXGotoJoin.cpp
  GenXGEPLowering.cpp
  GenXIMadPostLegalization.cpp
  GenXIntrinsics.cpp
  GenXLegalization.cpp
  GenXLiveRanges.cpp
  GenXLiveness.cpp
  GenXLowering.cpp
  GenXEmulate.cpp
  GenXModule.cpp
  GenXNumbering.cpp
  GenXPatternMatch.cpp
  GenXPostLegalization.cpp
  GenXPrinter.cpp
  GenXPressureTracker.cpp
  GenXPromotePredicate.cpp
  GenXRawSendRipper.cpp
  GenXReduceIntSize.cpp
  GenXRegion.cpp
  GenXRegionCollapsing.cpp
  GenXRematerialization.cpp
  GenXSimdCFConformance.cpp
  GenXSubtarget.cpp
  GenXTargetMachine.cpp
  GenXTidyControlFlow.cpp
  GenXUnbaling.cpp
  GenXUtil.cpp
  GenXVectorDecomposer.cpp
  GenXVisaFuncWriter.cpp
  GenXVisaRegAlloc.cpp
  GenXVisaWriter.cpp
  )

add_subdirectory(TargetInfo)

set(LLVM_TARGET_DEFINITIONS GenX.td)

tablegen(LLVM GenXGenSubtargetInfo.inc -gen-subtarget)
add_public_tablegen_target(GenXCommonTableGen)

add_dependencies(LLVMGenXCodeGen GenXCommonTableGen)
