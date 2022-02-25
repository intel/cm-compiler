/*========================== begin_copyright_notice ============================

Copyright (C) 2020-2021 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#ifdef LSCINC

// !!! --- Keep in sync with implementation header begin --- !!!
// TODO: we shall generate at least this stuff

enum class LSC_SubOpcode : uint8_t {
  LSC_LOAD = 0x00,
  LSC_LOAD_STRIDED = 0x01, // aka "load_block"
  LSC_LOAD_QUAD = 0x02, // aka "load_cmask"
  LSC_LOAD_BLOCK2D = 0x03,
  LSC_STORE = 0x04,
  LSC_STORE_STRIDED = 0x05, // aka "load_block"
  LSC_STORE_QUAD = 0x06, // aka "store_cmask"
  LSC_STORE_BLOCK2D = 0x07,
  //
  LSC_ATOMIC_IINC = 0x08,
  LSC_ATOMIC_IDEC = 0x09,
  LSC_ATOMIC_LOAD = 0x0A,
  LSC_ATOMIC_STORE = 0x0B,
  LSC_ATOMIC_IADD = 0x0C,
  LSC_ATOMIC_ISUB = 0x0D,
  LSC_ATOMIC_SMIN = 0x0E,
  LSC_ATOMIC_SMAX = 0x0F,
  LSC_ATOMIC_UMIN = 0x10,
  LSC_ATOMIC_UMAX = 0x11,
  LSC_ATOMIC_ICAS = 0x12,
  LSC_ATOMIC_FADD = 0x13,
  LSC_ATOMIC_FSUB = 0x14,
  LSC_ATOMIC_FMIN = 0x15,
  LSC_ATOMIC_FMAX = 0x16,
  LSC_ATOMIC_FCAS = 0x17,
  LSC_ATOMIC_AND = 0x18,
  LSC_ATOMIC_OR = 0x19,
  LSC_ATOMIC_XOR = 0x1A,
  //
  LSC_LOAD_STATUS = 0x1B,
  LSC_STORE_UNCOMPRESSED = 0x1C,
  LSC_CCS_UPDATE = 0x1D,
  LSC_READ_STATE_INFO = 0x1E,
  LSC_FENCE = 0x1F,
};

// L1 or L3 cache hint kinds.
enum class CacheHint : uint8_t {
  Default = 0,
  Uncached = 1,
  Cached = 2,
  WriteBack = 3,
  WriteThrough = 4,
  Streaming = 5,
  ReadInvalidate = 6,
};

// Data size or format to read or store.
enum class DataSize : uint8_t {
  U8 = 1,
  U16 = 2,
  U32 = 3,
  U64 = 4,
  U8U32 = 5,  // load 8b, zero extend to 32b; store the opposite
  U16U32 = 6, // load 8b, zero extend to 32b; store the opposite
  U16U32H = 7 // load 16b into high 16 of each 32b; store the high 16
};

// The number of elements to load per address (vector size)
enum class VectorSize : uint8_t {
  N1 = 1,  // 1 element
  N2 = 2,  // 2 element
  N3 = 3,  // 3 element
  N4 = 4,  // 4 element
  N8 = 5,  // 8 element
  N16 = 6, // 16 element
  N32 = 7, // 32 element
  N64 = 8  // 64 element
};

enum class LSC_DATA_ORDER : uint8_t {
  LSC_DATA_ORDER_INVALID,
  LSC_DATA_ORDER_NONTRANSPOSE,
  LSC_DATA_ORDER_TRANSPOSE,
};

enum class LSC_SCOPE : uint8_t {
  LSC_SCOPE_GROUP,
  LSC_SCOPE_LOCAL,
  LSC_SCOPE_TILE,
  LSC_SCOPE_GPU,
  LSC_SCOPE_GPUS,
  LSC_SCOPE_SYSTEM,
  LSC_SCOPE_SYSACQ
};

enum class LSC_FENCE_OP : uint8_t {
  LSC_FENCE_OP_NONE,
  LSC_FENCE_OP_EVICT,
  LSC_FENCE_OP_INVALIDATE,
  LSC_FENCE_OP_DISCARD,
  LSC_FENCE_OP_CLEAN,
  LSC_FENCE_OP_FLUSHL3
};
enum class LSC_SFID : uint8_t {
   LSC_UGM,
   LSC_UGML,
   LSC_TGM,
   LSC_SLM
};
// !!! --- Keep in sync with implementation header end --- !!!

// forward decl for static functions

static bool getBlock(CMBuiltinKind Kind);

// configuration for LSC templates
//
// Basically we have one format of intrinsics:
// template parameters:
// * optional RetTy param somewhere
// * optional N param somewhere
// * optional Transposed param somewhere
// * common block of (DS, VS, ImmOffset, L1H, L3H) at some offset
// * probably atomic Op value
// non-template parameters
// * return value
// * Idx (or Addr)
// * Offsets
// * Data (stores only)
// * Pred
// Idea is to abstract this to handle for any LSC message in uniform way and not
// duplicate code

// config[XXX_IDX] is place in template params to look for XXX
constexpr int TY_IDX = 0;
constexpr int DS_IDX = 1;
constexpr int VS_IDX = 2;
constexpr int IMOFF_IDX = 3;
constexpr int L1_IDX = 4;
constexpr int L3_IDX = 5;
constexpr int TRANSPOSED_IDX = 6;
constexpr int N_IDX = 7;
constexpr int OP_IDX = 8;
constexpr int ILLEGAL_TEMPLATE = 9;
// config[XXX_AIDX] is place in non-template params to look for XXX
constexpr int IDX_AIDX = 9;
constexpr int OFFSET_AIDX = 10;
constexpr int DATA_AIDX = 11;
constexpr int DATA1_AIDX = 12;
constexpr int PRED_AIDX = 13;
constexpr int CHMASK_AIDX = 14;
constexpr int ILLEGAL_NONTEMPLATE = 15;

// data structure for basic LSC parameters
struct LSCParams {
  llvm::Value *Ty_;
  const FunctionDecl *FD_;
  llvm::CallInst *CI_;
  const int *Config_;
  unsigned char DS_;         // data size
  unsigned char VS_;         // vector size
  int ImmOffset_ = 0;        // immediate offset
  unsigned char L1H_;        // L1 chache hint
  unsigned char L3H_;        // L3 chache hint
  unsigned char Transposed_; // is transposed access
  int N_;
  unsigned char Op_;

  llvm::Value *Idx_ = nullptr;
  llvm::Value *Offset_ = nullptr; // offset(s)
  llvm::Value *Pred_ = nullptr;   // predicate(s)
  llvm::Value *Data_ = nullptr;   // src0
  llvm::Value *Data1_ = nullptr;  // src1 if any
  llvm::Value *ChM_ = nullptr;    // channel mask

  template <typename T> void getTemplateIntParam(T &Data, unsigned N) {
    assert(N < ILLEGAL_TEMPLATE);
    if (Config_[N] == -1)
      return;
    Data = getIntegralValue(FD_, Config_[N]);
  }

  llvm::Value *getNonTemplateValue(unsigned N) {
    assert(N < ILLEGAL_NONTEMPLATE);
    int OpNum = Config_[N];
    // any field may exist or not
    if (OpNum < 0)
      return nullptr;
    // sources may exist or not in atomic
    if (CI_->getNumArgOperands() <= static_cast<unsigned>(OpNum))
      return nullptr;
    return CI_->getArgOperand(OpNum);
  }

  LSCParams(CGCMRuntime::CMCallInfo &CallInfo, const int *Config, CMBuiltinKind Kind)
      : FD_(CallInfo.CE->getDirectCallee()), CI_(CallInfo.CI), Config_(Config) {
    assert(CI_ && "Callinst required");
    assert(FD_ && "Func decl required");

    // start template params
    getTemplateIntParam(DS_, DS_IDX);
    getTemplateIntParam(VS_, VS_IDX);
    getTemplateIntParam(ImmOffset_, IMOFF_IDX);
    getTemplateIntParam(L1H_, L1_IDX);
    getTemplateIntParam(L3H_, L3_IDX);
    getTemplateIntParam(N_, N_IDX);
    getTemplateIntParam(Op_, OP_IDX);

    // defaults (mostly for SLM)
    if (Config_[L1_IDX] == -1)
      L1H_ = static_cast<unsigned char>(CacheHint::Default);
    if (Config_[L3_IDX] == -1)
      L3H_ = static_cast<unsigned char>(CacheHint::Default);

    // special logic for transpose
    int TVal = getBlock(Kind);
    getTemplateIntParam(TVal, TRANSPOSED_IDX);
    LSC_DATA_ORDER Transposed = TVal ?
        LSC_DATA_ORDER::LSC_DATA_ORDER_TRANSPOSE :
        LSC_DATA_ORDER::LSC_DATA_ORDER_NONTRANSPOSE;
    Transposed_ = static_cast<unsigned char>(Transposed);

    // start non-template params
    Idx_ = getNonTemplateValue(IDX_AIDX);
    Offset_ = getNonTemplateValue(OFFSET_AIDX);
    Data_ = getNonTemplateValue(DATA_AIDX);
    Data1_ = getNonTemplateValue(DATA1_AIDX);
    Pred_ = getNonTemplateValue(PRED_AIDX);
    ChM_ = getNonTemplateValue(CHMASK_AIDX);
  }

  void setNonTranspose() {
     auto Transposed = LSC_DATA_ORDER::LSC_DATA_ORDER_NONTRANSPOSE;
     Transposed_ = static_cast<unsigned char>(Transposed);
  }

  void setOp(LSC_SubOpcode Op) {
    Op_ = static_cast<unsigned char>(Op);
  }
};

// type of operation
enum LDTYPE { LOAD, STORE, PREFETCH, ATOMIC };

// type of surface
enum SFTYPE { BTI, FLAT, SLM };

/// \brief Emit LSC load, store and prefetch (BTI-based)
///
/// template <typename RetTy,
///           DataSize DS,
///           VectorSize VS,
///           int ImmOffset,
///           CacheHint L1H,
///           CacheHint L3H,
///           bool Transposed,
///           int N>
/// RetTy __cm_intrinsic_impl_load_bti(SurfaceIndex Idx,
///                                    vector<uint, 32> Offset,
///                                    vector<ushort, 32> Pred);

/* TY, DS, VS, IMOFF, L1, L3, TRANS, N, OP, IDX, OFF, DATA, DATA1, PRED, CHMASK */
constexpr int Lsc_load_bti[] = {0, 1, 2, 3, 4, 5, 6, 7, -1, 0, 1, -1, -1, 2, -1};
constexpr int Lsc_load_flat[] = {0, 1, 2, 3, 4, 5, 6, 7, -1, 0, 1, -1, -1, 2, -1};
constexpr int Lsc_load_slm[] = {0, 1, 2, 3, -1, -1, 4, 5, -1, -1, 0, -1, -1, 1, -1};

constexpr int Lsc_load4_bti[] = {0, 1, 2, 3, 4, 5, 6, 7, -1, 0, 1, -1, -1, 2, 3};
constexpr int Lsc_load4_flat[] = {0, 1, 2, 3, 4, 5, 6, 7, -1, 0, 1, -1, -1, 2, 3};
constexpr int Lsc_load4_slm[] = {0, 1, 2, 3, -1, -1, 4, 5, -1, -1, 0, -1, -1, 1, 2};

/// template <typename RetTy,
///           DataSize DS,
///           VectorSize VS,
///           int ImmOffset,
///           CacheHint L1H,
///           CacheHint L3H,
///           bool Transposed>
/// RetTy __cm_intrinsic_impl_block_load_bti(SurfaceIndex Idx,
///                                          uint Offset);

/* TY, DS, VS, IMOFF, L1, L3, TRANS, N, OP, IDX, OFF, DATA, DATA1, PRED, CHMASK */
constexpr int Lsc_block_load_bti[] = {0, 1, 2, 3, 4, 5, 6, -1, -1, 0, 1, -1, -1, -1, -1};
constexpr int Lsc_block_load_flat[] = {0, 1, 2, 3, 4, 5, 6, -1, -1, 0, 1, -1, -1, -1, -1};
constexpr int Lsc_block_load_slm[] = {0, 1, 2, 3, -1, -1, 4, -1, -1, -1, 0, -1, -1, -1, -1};

/// template <DataSize DS,
///           VectorSize VS,
///           int ImmOffset,
///           CacheHint L1H,
///           CacheHint L3H,
///           int N>
/// void __cm_intrinsic_impl_prefetch_bti(SurfaceIndex Idx,
///                                       vector<uint, N> Offset,
///                                       vector<ushort, N> Pred);

/* TY, DS, VS, IMOFF, L1, L3, TRANS, N, OP, IDX, OFF, DATA, DATA1, PRED, CHMASK */
constexpr int Lsc_prefetch_bti[] = {-1, 0, 1, 2, 3, 4, -1, 5, -1, 0, 1, -1, -1, 2, -1};
constexpr int Lsc_prefetch_flat[] = {-1, 0, 1, 2, 3, 4, -1, 5, -1, 0, 1, -1, -1, 2, -1};

///
/// template <DataSize DS,
///           VectorSize VS,
///           int ImmOffset,
///           CacheHint L1H,
///           CacheHint L3H>
///  void __cm_intrinsic_impl_block_prefetch_bti(SurfaceIndex Idx,
///                                              unsigned Offset);

/* TY, DS, VS, IMOFF, L1, L3, TRANS, N, OP, IDX, OFF, DATA, DATA1, PRED, CHMASK */
constexpr int Lsc_block_prefetch_bti[] = {-1, 0,  1, 2, 3,  4, -1, -1, -1, 0, 1, -1, -1, -1, -1};
constexpr int Lsc_block_prefetch_flat[] = {-1, 0,  1, 2, 3,  4, -1, -1, -1, 0, 1, -1, -1, -1, -1};

/// template <typename T,
///           DataSize DS,
///           VectorSize VS,
///           int ImmOffset,
///           CacheHint L1H,
///           CacheHint L3H,
///           bool Transposed,
///           int N>
/// void __cm_intrinsic_impl_store_bti(SurfaceIndex Idx,
///                                    vector<uint, N> Offset,
///                                    vector<T, N * VS.size()> Data,
///                                    vector<ushort, N> Pred);

/* TY, DS, VS, IMOFF, L1, L3, TRANS, N, OP, IDX, OFF, DATA, DATA1, PRED, CHMASK */
constexpr int Lsc_store_bti[] = {0, 1, 2, 3, 4, 5, 6, 7, -1, 0, 1, 2, -1, 3, -1};
constexpr int Lsc_store_flat[] = {0, 1, 2, 3, 4, 5, 6, 7, -1, 0, 1, 2, -1, 3, -1};
constexpr int Lsc_store_slm[] = {0, 1, 2, 3, -1, -1, 4, 5, -1, -1, 0, 1, -1, 2, -1};

constexpr int Lsc_store4_bti[] = {0, 1, 2, 3, 4, 5, 6, 7, -1, 0, 1, 2, -1, 3, 4};
constexpr int Lsc_store4_flat[] = {0, 1, 2, 3, 4, 5, 6, 7, -1, 0, 1, 2, -1, 3, 4};
constexpr int Lsc_store4_slm[] = {0, 1, 2, 3, -1, -1, 4, 5, -1, -1, 0, 1, -1, 2, 3};

/// template <typename T,
///           DataSize DS,
///           VectorSize VS,
///           int ImmOffset,
///           CacheHint L1H,
///           CacheHint L3H,
///           bool Transposed,
/// void __cm_intrinsic_impl_block_store_bti(SurfaceIndex Idx,
///                                          uint Offset,
///                                          vector<T, VS.size()> Data)

/* TY, DS, VS, IMOFF, L1, L3, TRANS, N, OP, IDX, OFF, DATA, DATA1, PRED, CHMASK */
constexpr int Lsc_block_store_bti[] = {0, 1, 2, 3, 4, 5, 6, -1, -1, 0, 1, 2, -1, -1, -1};
constexpr int Lsc_block_store_flat[] = {0, 1, 2, 3, 4, 5, 6, -1, -1, 0, 1, 2, -1, -1, -1};
constexpr int Lsc_block_store_slm[] = {0, 1, 2, 3, -1, -1, 4, -1, -1, -1, 0, 1, -1, -1, -1};

// template <AtomicOp Op,
//           DataSize DS,
//           VectorSize VS,
//           bool Transposed,
//           CacheHint L1H,
//           CacheHint L3H,
//           typename RetTy,
//           typename... Args>
// RetTy _cm_intrinsic_impl_lsc_atomic_bti(vector<ushort, 32> Pred,
//                                         SurfaceIndex Idx,
//                                         vector<unsigned, 32> Offset,
//                                         Args... args);

/* TY, DS, VS, IMOFF, L1, L3, TRANS, N, OP, IDX, OFF, DATA, DATA1, PRED, CHMASK */
constexpr int Lsc_atomic_bti[] = {6, 1, 2, -1, 4, 5, 3, -1, 0, 1, 2, 3, 4, 0, -1};
constexpr int Lsc_atomic_flat[] = {6, 1, 2, -1, 4, 5, 3, -1, 0, 1, 2, 3, 4, 0, -1};
constexpr int Lsc_atomic_slm[] = {6, 1, 2, -1, 4, 5, 3, -1, 0, -1, 1, 2, 3, 0, -1};

// intrinsics from kind
static int getLSCIntrinsic(CMBuiltinKind Kind) {
  switch (Kind) {
  case CMBK_cm_load_impl:
  case CMBK_cm_block_load_impl:
    return llvm::GenXIntrinsic::genx_lsc_load_bti;
  case CMBK_cm_load4_impl:
    return llvm::GenXIntrinsic::genx_lsc_load_quad_bti;
  case CMBK_cm_load_flat_impl:
  case CMBK_cm_block_load_flat_impl:
    return llvm::GenXIntrinsic::genx_lsc_load_stateless;
  case CMBK_cm_load4_flat_impl:
    return llvm::GenXIntrinsic::genx_lsc_load_quad_stateless;
  case CMBK_cm_load_slm_impl:
  case CMBK_cm_block_load_slm_impl:
    return llvm::GenXIntrinsic::genx_lsc_load_slm;
  case CMBK_cm_load4_slm_impl:
    return llvm::GenXIntrinsic::genx_lsc_load_quad_slm;
  case CMBK_cm_store_impl:
  case CMBK_cm_block_store_impl:
    return llvm::GenXIntrinsic::genx_lsc_store_bti;
  case CMBK_cm_store4_impl:
    return llvm::GenXIntrinsic::genx_lsc_store_quad_bti;
  case CMBK_cm_store_flat_impl:
  case CMBK_cm_block_store_flat_impl:
    return llvm::GenXIntrinsic::genx_lsc_store_stateless;
  case CMBK_cm_store4_flat_impl:
    return llvm::GenXIntrinsic::genx_lsc_store_quad_stateless;
  case CMBK_cm_store_slm_impl:
  case CMBK_cm_block_store_slm_impl:
    return llvm::GenXIntrinsic::genx_lsc_store_slm;
  case CMBK_cm_store4_slm_impl:
    return llvm::GenXIntrinsic::genx_lsc_store_quad_slm;
  case CMBK_cm_prefetch_impl:
  case CMBK_cm_block_prefetch_impl:
    return llvm::GenXIntrinsic::genx_lsc_prefetch_bti;
  case CMBK_cm_prefetch_flat_impl:
  case CMBK_cm_block_prefetch_flat_impl:
    return llvm::GenXIntrinsic::genx_lsc_prefetch_stateless;
  case CMBK_cm_atomic_bti_impl:
    return llvm::GenXIntrinsic::genx_lsc_xatomic_bti;
  case CMBK_cm_atomic_flat_impl:
    return llvm::GenXIntrinsic::genx_lsc_xatomic_stateless;
  case CMBK_cm_atomic_slm_impl:
    return llvm::GenXIntrinsic::genx_lsc_xatomic_slm;
  default:
    assert(0 && "Kind not supported");
  }
}

// config from kind
static const int *getConfig(CMBuiltinKind Kind) {
  switch (Kind) {
  case CMBK_cm_prefetch_impl: return Lsc_prefetch_bti;
  case CMBK_cm_block_prefetch_impl: return Lsc_block_prefetch_bti;
  case CMBK_cm_prefetch_flat_impl: return Lsc_prefetch_flat;
  case CMBK_cm_block_prefetch_flat_impl: return Lsc_block_prefetch_flat;
  case CMBK_cm_load_impl: return Lsc_load_bti;
  case CMBK_cm_load4_impl: return Lsc_load4_bti;
  case CMBK_cm_block_load_impl: return Lsc_block_load_bti;
  case CMBK_cm_load_flat_impl: return Lsc_load_flat;
  case CMBK_cm_load4_flat_impl: return Lsc_load4_flat;
  case CMBK_cm_block_load_flat_impl: return Lsc_block_load_flat;
  case CMBK_cm_load_slm_impl: return Lsc_load_slm;
  case CMBK_cm_load4_slm_impl: return Lsc_load4_slm;
  case CMBK_cm_block_load_slm_impl: return Lsc_block_load_slm;
  case CMBK_cm_store_impl: return Lsc_store_bti;
  case CMBK_cm_store4_impl: return Lsc_store4_bti;
  case CMBK_cm_block_store_impl: return Lsc_block_store_bti;
  case CMBK_cm_store_flat_impl: return Lsc_store_flat;
  case CMBK_cm_store4_flat_impl: return Lsc_store4_flat;
  case CMBK_cm_block_store_flat_impl: return Lsc_block_store_flat;
  case CMBK_cm_store_slm_impl: return Lsc_store_slm;
  case CMBK_cm_store4_slm_impl: return Lsc_store4_slm;
  case CMBK_cm_block_store_slm_impl: return Lsc_block_store_slm;
  case CMBK_cm_atomic_bti_impl: return Lsc_atomic_bti;
  case CMBK_cm_atomic_flat_impl: return Lsc_atomic_flat;
  case CMBK_cm_atomic_slm_impl: return Lsc_atomic_slm;
  default:
    assert(0 && "Not a valid builtin");
  }
}

// subop from kind (except atomics)
static LSC_SubOpcode getSubOp(CMBuiltinKind Kind) {
  switch (Kind) {
  case CMBK_cm_prefetch_impl:
  case CMBK_cm_block_prefetch_impl:
  case CMBK_cm_prefetch_flat_impl:
  case CMBK_cm_block_prefetch_flat_impl:
  case CMBK_cm_load_impl:
  case CMBK_cm_block_load_impl:
  case CMBK_cm_load_flat_impl:
  case CMBK_cm_block_load_flat_impl:
  case CMBK_cm_load_slm_impl:
  case CMBK_cm_block_load_slm_impl:
     return LSC_SubOpcode::LSC_LOAD;
  case CMBK_cm_load4_impl:
  case CMBK_cm_load4_flat_impl:
  case CMBK_cm_load4_slm_impl:
     return LSC_SubOpcode::LSC_LOAD_QUAD;
  case CMBK_cm_store_impl:
  case CMBK_cm_block_store_impl:
  case CMBK_cm_store_flat_impl:
  case CMBK_cm_block_store_flat_impl:
  case CMBK_cm_store_slm_impl:
  case CMBK_cm_block_store_slm_impl:
     return LSC_SubOpcode::LSC_STORE;
  case CMBK_cm_store4_impl:
  case CMBK_cm_store4_flat_impl:
  case CMBK_cm_store4_slm_impl:
     return LSC_SubOpcode::LSC_STORE_QUAD;
  default:
    assert(0 && "Not a valid builtin");
  }
}

// optype from kind
static LDTYPE getOpType(CMBuiltinKind Kind) {
  switch (Kind) {
  case CMBK_cm_prefetch_impl:
  case CMBK_cm_block_prefetch_impl:
  case CMBK_cm_prefetch_flat_impl:
  case CMBK_cm_block_prefetch_flat_impl:
    return PREFETCH;
  case CMBK_cm_load_impl:
  case CMBK_cm_block_load_impl:
  case CMBK_cm_load_flat_impl:
  case CMBK_cm_block_load_flat_impl:
  case CMBK_cm_load_slm_impl:
  case CMBK_cm_block_load_slm_impl:
  case CMBK_cm_load4_impl:
  case CMBK_cm_load4_flat_impl:
  case CMBK_cm_load4_slm_impl:
     return LOAD;
  case CMBK_cm_store_impl:
  case CMBK_cm_block_store_impl:
  case CMBK_cm_store_flat_impl:
  case CMBK_cm_block_store_flat_impl:
  case CMBK_cm_store_slm_impl:
  case CMBK_cm_block_store_slm_impl:
  case CMBK_cm_store4_impl:
  case CMBK_cm_store4_flat_impl:
  case CMBK_cm_store4_slm_impl:
     return STORE;
  case CMBK_cm_atomic_bti_impl:
  case CMBK_cm_atomic_flat_impl:
  case CMBK_cm_atomic_slm_impl:
    return ATOMIC;
  default:
    assert(0 && "Not a valid builtin");
  }
}

// surface from kind
static SFTYPE getSFType(CMBuiltinKind Kind) {
  switch (Kind) {
  case CMBK_cm_prefetch_impl:
  case CMBK_cm_block_prefetch_impl:
  case CMBK_cm_load_impl:
  case CMBK_cm_block_load_impl:
  case CMBK_cm_store_impl:
  case CMBK_cm_block_store_impl:
  case CMBK_cm_atomic_bti_impl:
  case CMBK_cm_load4_impl:
  case CMBK_cm_store4_impl:
    return BTI;
  case CMBK_cm_prefetch_flat_impl:
  case CMBK_cm_block_prefetch_flat_impl:
  case CMBK_cm_load_flat_impl:
  case CMBK_cm_block_load_flat_impl:
  case CMBK_cm_store_flat_impl:
  case CMBK_cm_block_store_flat_impl:
  case CMBK_cm_atomic_flat_impl:
  case CMBK_cm_load4_flat_impl:
  case CMBK_cm_store4_flat_impl:
    return FLAT;
  case CMBK_cm_load_slm_impl:
  case CMBK_cm_block_load_slm_impl:
  case CMBK_cm_store_slm_impl:
  case CMBK_cm_block_store_slm_impl:
  case CMBK_cm_atomic_slm_impl:
  case CMBK_cm_store4_slm_impl:
  case CMBK_cm_load4_slm_impl:
    return SLM;
  default:
    assert(0 && "Not a valid builtin");
  }
}

// is block or not
static bool getBlock(CMBuiltinKind Kind) {
  switch (Kind) {
  case CMBK_cm_prefetch_impl:
  case CMBK_cm_prefetch_flat_impl:
  case CMBK_cm_load_impl:
  case CMBK_cm_load_flat_impl:
  case CMBK_cm_store_impl:
  case CMBK_cm_store_flat_impl:
  case CMBK_cm_load_slm_impl:
  case CMBK_cm_store_slm_impl:
  case CMBK_cm_atomic_bti_impl:
  case CMBK_cm_atomic_flat_impl:
  case CMBK_cm_atomic_slm_impl:
  case CMBK_cm_load4_impl:
  case CMBK_cm_load4_flat_impl:
  case CMBK_cm_load4_slm_impl:
  case CMBK_cm_store4_impl:
  case CMBK_cm_store4_flat_impl:
  case CMBK_cm_store4_slm_impl:
    return false;
  case CMBK_cm_block_prefetch_impl:
  case CMBK_cm_block_prefetch_flat_impl:
  case CMBK_cm_block_load_impl:
  case CMBK_cm_block_load_flat_impl:
  case CMBK_cm_block_store_impl:
  case CMBK_cm_block_store_flat_impl:
  case CMBK_cm_block_load_slm_impl:
  case CMBK_cm_block_store_slm_impl:
    return true;
  default:
    assert(0 && "Not a valid builtin");
  }
}

#endif
