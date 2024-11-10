
add_llvm_library( PreModelChecking MODULE
  PreModelCheckingWarpper.cpp
  ConstExprRemover.cpp
  RemoveUnusedInstructions.cpp
  GetElementPtrSimplifier.cpp
  SplitCalls.cpp
  BBReNamer.cpp
  InCallIDMetaData.cpp

  # ADDITIONAL_HEADER_DIRS
  # ${LLVM_MAIN_INCLUDE_DIR}/llvm/Transforms

  DEPENDS
  intrinsics_gen
  PLUGIN_TOOL
  opt

  # LINK_COMPONENTS
  # Core
  # Support
  # TransformUtils
  # Scalar
  # IPO
  # Passes
)
