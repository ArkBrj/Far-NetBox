#pragma once

#include <string>
#include <hash_map.h>
#include "nbglobals.h"
//#include "Global.h"

class TObject;
class THashTable;

enum TObjectClassId
{
  OBJECT_CLASS_TObject = 1,
  OBJECT_CLASS_Exception,
  OBJECT_CLASS_ExtException,
  OBJECT_CLASS_EConvertError,
  OBJECT_CLASS_EStreamError,
  OBJECT_CLASS_EAbort,
  OBJECT_CLASS_EAccessViolation,
  OBJECT_CLASS_ESsh,
  OBJECT_CLASS_ETerminal,
  OBJECT_CLASS_ECommand,
  OBJECT_CLASS_EScp,
  OBJECT_CLASS_ESkipFile,
  OBJECT_CLASS_EFileSkipped,
  OBJECT_CLASS_EOSExtException,
  OBJECT_CLASS_ECRTExtException,
  OBJECT_CLASS_EFatal,
  OBJECT_CLASS_ESshFatal,
  OBJECT_CLASS_ESshTerminate,
  OBJECT_CLASS_ECallbackGuardAbort,
  OBJECT_CLASS_EFileNotFoundError,
  OBJECT_CLASS_EOSError,
  OBJECT_CLASS_TPersistent,
  OBJECT_CLASS_TStrings,
  OBJECT_CLASS_TNamedObject,
  OBJECT_CLASS_TSessionData,
  OBJECT_CLASS_TDateTime,
  OBJECT_CLASS_TSHFileInfo,
  OBJECT_CLASS_TStream,
  OBJECT_CLASS_TRegistry,
  OBJECT_CLASS_TShortCut,
  OBJECT_CLASS_TDelphiSet,
  OBJECT_CLASS_TFormatSettings,
  OBJECT_CLASS_TCriticalSection,
  OBJECT_CLASS_TBookmarks,
  OBJECT_CLASS_TDateTimeParams,
  OBJECT_CLASS_TGuard,
  OBJECT_CLASS_TUnguard,
  OBJECT_CLASS_TValueRestorer,
  OBJECT_CLASS_TConfiguration,
  OBJECT_CLASS_TShortCuts,
  OBJECT_CLASS_TCopyParamType,
  OBJECT_CLASS_TFileBuffer,
  OBJECT_CLASS_TFileMasks,
  OBJECT_CLASS_TParams,
  OBJECT_CLASS_TFile,
  OBJECT_CLASS_TRemoteFile,
  OBJECT_CLASS_TRemoteDirectoryFile,
  OBJECT_CLASS_TRemoteDirectory,
  OBJECT_CLASS_TRemoteParentDirectory,
  OBJECT_CLASS_TList,
  OBJECT_CLASS_TObjectList,
  OBJECT_CLASS_TStringList,
  OBJECT_CLASS_TBookmarkList,
  OBJECT_CLASS_TBookmark,
  OBJECT_CLASS_TCustomCommand,
  OBJECT_CLASS_TCustomCommandData,
  OBJECT_CLASS_TFileOperationProgressType,
  OBJECT_CLASS_TSuspendFileOperationProgress,
  OBJECT_CLASS_TSinkFileParams,
  OBJECT_CLASS_TFileTransferData,
  OBJECT_CLASS_TClipboardHandler,
  OBJECT_CLASS_TOverwriteFileParams,
  OBJECT_CLASS_TOpenRemoteFileParams,
  OBJECT_CLASS_TCustomFileSystem,
  OBJECT_CLASS_TFTPFileSystem,
  OBJECT_CLASS_TSCPFileSystem,
  OBJECT_CLASS_TSFTPFileSystem,
  OBJECT_CLASS_TWebDAVFileSystem,
  OBJECT_CLASS_TMessageQueue,
  OBJECT_CLASS_TFTPFileListHelper,
  OBJECT_CLASS_THierarchicalStorage,
  OBJECT_CLASS_TQueryButtonAlias,
  OBJECT_CLASS_TQueryParams,
  OBJECT_CLASS_TNamedObjectList,
  OBJECT_CLASS_TStoredSessionList,
  OBJECT_CLASS_TOptions,
  OBJECT_CLASS_TUserAction,
  OBJECT_CLASS_TSimpleThread,
  OBJECT_CLASS_TKeepAliveThread,
  OBJECT_CLASS_TTunnelThread,
  OBJECT_CLASS_TSignalThread,
  OBJECT_CLASS_TTerminalThread,
  OBJECT_CLASS_TTerminalQueue,
  OBJECT_CLASS_TTerminalItem,
  OBJECT_CLASS_TQueueItem,
  OBJECT_CLASS_TLocatedQueueItem,
  OBJECT_CLASS_TTransferQueueItem,
  OBJECT_CLASS_TUploadQueueItem,
  OBJECT_CLASS_TDownloadQueueItem,
  OBJECT_CLASS_TInfo,
  OBJECT_CLASS_TQueueItemProxy,
  OBJECT_CLASS_TTerminalQueueStatus,
  OBJECT_CLASS_TRemoteToken,
  OBJECT_CLASS_TRemoteTokenList,
  OBJECT_CLASS_TRemoteFileList,
  OBJECT_CLASS_TRights,
  OBJECT_CLASS_TValidProperties,
  OBJECT_CLASS_TRemoteProperties,
  OBJECT_CLASS_TCommandSet,
  OBJECT_CLASS_TPoolForDataEvent,
  OBJECT_CLASS_TSecureShell,
  OBJECT_CLASS_TIEProxyConfig,
  OBJECT_CLASS_TSessionActionRecord,
  OBJECT_CLASS_TSessionInfo,
  OBJECT_CLASS_TFileSystemInfo,
  OBJECT_CLASS_TSessionAction,
  OBJECT_CLASS_TActionLog,
  OBJECT_CLASS_TSFTPSupport,
  OBJECT_CLASS_TSFTPPacket,
  OBJECT_CLASS_TSFTPQueuePacket,
  OBJECT_CLASS_TSFTPQueue,
  OBJECT_CLASS_TSFTPBusy,
  OBJECT_CLASS_TLoopDetector,
  OBJECT_CLASS_TMoveFileParams,
  OBJECT_CLASS_TFilesFindParams,
  OBJECT_CLASS_TCallbackGuard,
  OBJECT_CLASS_TOutputProxy,
  OBJECT_CLASS_TSynchronizeFileData,
  OBJECT_CLASS_TSynchronizeData,
  OBJECT_CLASS_TSessionUI,
  OBJECT_CLASS_TTunnelUI,
  OBJECT_CLASS_TTerminal,
  OBJECT_CLASS_TSecondaryTerminal,
  OBJECT_CLASS_TBackgroundTerminal,
  OBJECT_CLASS_TTerminalList,
  OBJECT_CLASS_TCustomCommandParams,
  OBJECT_CLASS_TCalculateSizeStats,
  OBJECT_CLASS_TCalculateSizeParams,
  OBJECT_CLASS_TMakeLocalFileListParams,
  OBJECT_CLASS_TSynchronizeOptions,
  OBJECT_CLASS_TSynchronizeChecklist,
  OBJECT_CLASS_TItem,
  OBJECT_CLASS_TChecklistItem,
  OBJECT_CLASS_TFileInfo,
  OBJECT_CLASS_TSpaceAvailable,
  OBJECT_CLASS_TWebDAVFileListHelper,
  OBJECT_CLASS_TCFileZillaApi,
  OBJECT_CLASS_TFTPServerCapabilities,
  OBJECT_CLASS_TCFileZillaTools,
  OBJECT_CLASS_T_directory,
  OBJECT_CLASS_T_direntry,
  OBJECT_CLASS_TCopyParamRuleData,
  OBJECT_CLASS_TCopyParamRule,
  OBJECT_CLASS_TCopyParamList,
  OBJECT_CLASS_TProgramParamsOwner,
  OBJECT_CLASS_TSynchronizeParamType,
  OBJECT_CLASS_TSynchronizeController,
  OBJECT_CLASS_TFarDialog,
  OBJECT_CLASS_TFarDialogContainer,
  OBJECT_CLASS_TRightsContainer,
  OBJECT_CLASS_TCopyParamsContainer,
  OBJECT_CLASS_TFarDialogItem,
  OBJECT_CLASS_TConsoleTitleParam,
  OBJECT_CLASS_TFarMessageParams,
  OBJECT_CLASS_TCustomFarPlugin,
  OBJECT_CLASS_TCustomFarFileSystem,
  OBJECT_CLASS_TFarBox,
  OBJECT_CLASS_TFarButton,
  OBJECT_CLASS_TTabButton,
  OBJECT_CLASS_TFarList,
  OBJECT_CLASS_TFarMenuItems,
  OBJECT_CLASS_TWinSCPPlugin,
  OBJECT_CLASS_TWinSCPFileSystem,
  OBJECT_CLASS_TFarPanelModes,
  OBJECT_CLASS_TFarKeyBarTitles,
  OBJECT_CLASS_TCustomFarPanelItem,
  OBJECT_CLASS_TFarPanelInfo,
  OBJECT_CLASS_TFarEditorInfo,
  OBJECT_CLASS_TFarEnvGuard,
  OBJECT_CLASS_TFarListBox,
  OBJECT_CLASS_TFarEdit,
  OBJECT_CLASS_TLabelList,
  OBJECT_CLASS_TGUIConfiguration,
  OBJECT_CLASS_TFarConfiguration,
  OBJECT_CLASS_TGUICopyParamType,
  OBJECT_CLASS_TCNBFile,
  OBJECT_CLASS_TMultipleEdit,
  OBJECT_CLASS_TEditHistory,
  OBJECT_CLASS_TFarMessageData,
  OBJECT_CLASS_TMessageParams,
  OBJECT_CLASS_TFileZillaIntern,
  OBJECT_CLASS_CApiLog,
  OBJECT_CLASS_TFarPanelItem,
  OBJECT_CLASS_THintPanelItem,
  OBJECT_CLASS_TSessionPanelItem,
  OBJECT_CLASS_TSessionFolderPanelItem,
  OBJECT_CLASS_TRemoteFilePanelItem,
  OBJECT_CLASS_TFarText,
  OBJECT_CLASS_TFarCheckBox,
  OBJECT_CLASS_TFarComboBox,
  OBJECT_CLASS_TFarRadioButton,
  OBJECT_CLASS_TFarSeparator,
  OBJECT_CLASS_TFarLister,

};

//===-- from llvm/Support/Casting.h - Allow flexible, checked, casts
//
// This file defines the isa<X>(), cast<X>(), dyn_cast<X>(), cast_or_null<X>(),
// and dyn_cast_or_null<X>() templates.
//

#include "type_traits.h"

namespace rtti {

//===----------------------------------------------------------------------===//
//                          isa<x> Support Templates
//===----------------------------------------------------------------------===//

// Define a template that can be specialized by smart pointers to reflect the
// fact that they are automatically dereferenced, and are not involved with the
// template selection process...  the default implementation is a noop.
//
template<typename From> struct simplify_type {
  typedef       From SimpleType;        // The real type this represents...

  // An accessor to get the real value...
  static SimpleType &getSimplifiedValue(From &Val) { return Val; }
};

template<typename From> struct simplify_type<const From> {
  typedef typename simplify_type<From>::SimpleType NonConstSimpleType;
  typedef typename nb::add_const_past_pointer<NonConstSimpleType>::type
    SimpleType;
  typedef typename nb::add_lvalue_reference_if_not_pointer<SimpleType>::type
    RetType;
  static RetType getSimplifiedValue(const From& Val) {
    return simplify_type<From>::getSimplifiedValue(const_cast<From&>(Val));
  }
};

// The core of the implementation of isa<X> is here; To and From should be
// the names of classes.  This template can be specialized to customize the
// implementation of isa<> without rewriting it from scratch.
template <typename To, typename From, typename Enabler = void>
struct isa_impl {
  static inline bool doit(const From &Val) {
    return To::classof(&Val);
  }
};

/// \brief Always allow upcasts, and perform no dynamic check for them.
template <typename To, typename From>
struct isa_impl<
    To, From, typename std::enable_if<std::is_base_of<To, From>::value>::type> {
  static inline bool doit(const From &) { return true; }
};

template <typename To, typename From> struct isa_impl_cl {
  static inline bool doit(const From &Val) {
    return isa_impl<To, From>::doit(Val);
  }
};

template <typename To, typename From> struct isa_impl_cl<To, const From> {
  static inline bool doit(const From &Val) {
    return isa_impl<To, From>::doit(Val);
  }
};

template <typename To, typename From> struct isa_impl_cl<To, From*> {
  static inline bool doit(const From *Val) {
    assert(Val && "isa<> used on a null pointer");
    return isa_impl<To, From>::doit(*Val);
  }
};

template <typename To, typename From> struct isa_impl_cl<To, From*const> {
  static inline bool doit(const From *Val) {
    assert(Val && "isa<> used on a null pointer");
    return isa_impl<To, From>::doit(*Val);
  }
};

template <typename To, typename From> struct isa_impl_cl<To, const From*> {
  static inline bool doit(const From *Val) {
    assert(Val && "isa<> used on a null pointer");
    return isa_impl<To, From>::doit(*Val);
  }
};

template <typename To, typename From> struct isa_impl_cl<To, const From*const> {
  static inline bool doit(const From *Val) {
    assert(Val && "isa<> used on a null pointer");
    return isa_impl<To, From>::doit(*Val);
  }
};

template<typename To, typename From, typename SimpleFrom>
struct isa_impl_wrap {
  // When From != SimplifiedType, we can simplify the type some more by using
  // the simplify_type template.
  static bool doit(const From &Val) {
    return isa_impl_wrap<To, SimpleFrom,
      typename simplify_type<SimpleFrom>::SimpleType>::doit(
                          simplify_type<const From>::getSimplifiedValue(Val));
  }
};

template<typename To, typename FromTy>
struct isa_impl_wrap<To, FromTy, FromTy> {
  // When From == SimpleType, we are as simple as we are going to get.
  static bool doit(const FromTy &Val) {
    return isa_impl_cl<To,FromTy>::doit(Val);
  }
};

// isa<X> - Return true if the parameter to the template is an instance of the
// template type argument.  Used like this:
//
//  if (isa<Type>(myVal)) { ... }
//
template <class X, class Y>
inline bool isa(const Y &Val) {
  return isa_impl_wrap<X, const Y,
                       typename simplify_type<const Y>::SimpleType>::doit(Val);
}

//===----------------------------------------------------------------------===//
//                          cast<x> Support Templates
//===----------------------------------------------------------------------===//

template<class To, class From> struct cast_retty;


// Calculate what type the 'cast' function should return, based on a requested
// type of To and a source type of From.
template<class To, class From> struct cast_retty_impl {
  typedef To& ret_type;         // Normal case, return Ty&
};
template<class To, class From> struct cast_retty_impl<To, const From> {
  typedef const To &ret_type;   // Normal case, return Ty&
};

template<class To, class From> struct cast_retty_impl<To, From*> {
  typedef To* ret_type;         // Pointer arg case, return Ty*
};

template<class To, class From> struct cast_retty_impl<To, const From*> {
  typedef const To* ret_type;   // Constant pointer arg case, return const Ty*
};

template<class To, class From> struct cast_retty_impl<To, const From*const> {
  typedef const To* ret_type;   // Constant pointer arg case, return const Ty*
};


template<class To, class From, class SimpleFrom>
struct cast_retty_wrap {
  // When the simplified type and the from type are not the same, use the type
  // simplifier to reduce the type, then reuse cast_retty_impl to get the
  // resultant type.
  typedef typename cast_retty<To, SimpleFrom>::ret_type ret_type;
};

template<class To, class FromTy>
struct cast_retty_wrap<To, FromTy, FromTy> {
  // When the simplified type is equal to the from type, use it directly.
  typedef typename cast_retty_impl<To,FromTy>::ret_type ret_type;
};

template<class To, class From>
struct cast_retty {
  typedef typename cast_retty_wrap<To, From,
                   typename simplify_type<From>::SimpleType>::ret_type ret_type;
};

// Ensure the non-simple values are converted using the simplify_type template
// that may be specialized by smart pointers...
//
template<class To, class From, class SimpleFrom> struct cast_convert_val {
  // This is not a simple type, use the template to simplify it...
  static typename cast_retty<To, From>::ret_type doit(From &Val) {
    return cast_convert_val<To, SimpleFrom,
      typename simplify_type<SimpleFrom>::SimpleType>::doit(
                          simplify_type<From>::getSimplifiedValue(Val));
  }
};

template<class To, class FromTy> struct cast_convert_val<To,FromTy,FromTy> {
  // This _is_ a simple type, just cast it.
  static typename cast_retty<To, FromTy>::ret_type doit(const FromTy &Val) {
    typename cast_retty<To, FromTy>::ret_type Res2
     = (typename cast_retty<To, FromTy>::ret_type)const_cast<FromTy&>(Val);
    return Res2;
  }
};

template <class X> struct is_simple_type {
  static const bool value =
      std::is_same<X, typename simplify_type<X>::SimpleType>::value;
};

// cast<X> - Return the argument parameter cast to the specified type.  This
// casting operator asserts that the type is correct, so it does not return null
// on failure.  It does not allow a null argument (use cast_or_null for that).
// It is typically used like this:
//
//  cast<Instruction>(myVal)->getParent()
//
template <class X, class Y>
inline typename std::enable_if<!is_simple_type<Y>::value,
                               typename cast_retty<X, const Y>::ret_type>::type
cast(const Y &Val) {
  assert(isa<X>(Val) && "cast<Ty>() argument of incompatible type!");
  return cast_convert_val<
      X, const Y, typename simplify_type<const Y>::SimpleType>::doit(Val);
}

template <class X, class Y>
inline typename cast_retty<X, Y>::ret_type cast(Y &Val) {
  assert(isa<X>(Val) && "cast<Ty>() argument of incompatible type!");
  return cast_convert_val<X, Y,
                          typename simplify_type<Y>::SimpleType>::doit(Val);
}

template <class X, class Y>
inline typename cast_retty<X, Y *>::ret_type cast(Y *Val) {
  assert(isa<X>(Val) && "cast<Ty>() argument of incompatible type!");
  return cast_convert_val<X, Y*,
                          typename simplify_type<Y*>::SimpleType>::doit(Val);
}

// cast_or_null<X> - Functionally identical to cast, except that a null value is
// accepted.
//
template <class X, class Y>
inline typename std::enable_if<
    !is_simple_type<Y>::value, typename cast_retty<X, const Y>::ret_type>::type
cast_or_null(const Y &Val) {
  if (!Val)
    return nullptr;
  assert(isa<X>(Val) && "cast_or_null<Ty>() argument of incompatible type!");
  return cast<X>(Val);
}

template <class X, class Y>
inline typename std::enable_if<
    !is_simple_type<Y>::value, typename cast_retty<X, Y>::ret_type>::type
cast_or_null(Y &Val) {
  if (!Val)
    return nullptr;
  assert(isa<X>(Val) && "cast_or_null<Ty>() argument of incompatible type!");
  return cast<X>(Val);
}

template <class X, class Y>
inline typename cast_retty<X, Y *>::ret_type
cast_or_null(Y *Val) {
  if (!Val) return nullptr;
  assert(isa<X>(Val) && "cast_or_null<Ty>() argument of incompatible type!");
  return cast<X>(Val);
}


// dyn_cast<X> - Return the argument parameter cast to the specified type.  This
// casting operator returns null if the argument is of the wrong type, so it can
// be used to test for a type as well as cast if successful.  This should be
// used in the context of an if statement like this:
//
//  if (const Instruction *I = dyn_cast<Instruction>(myVal)) { ... }
//

template <class X, class Y>
inline typename std::enable_if<
    !is_simple_type<Y>::value, typename cast_retty<X, const Y>::ret_type>::type
dyn_cast(const Y &Val) {
  return isa<X>(Val) ? cast<X>(Val) : nullptr;
}

template <class X, class Y>
inline typename cast_retty<X, Y>::ret_type
dyn_cast(Y &Val) {
  return isa<X>(Val) ? cast<X>(Val) : nullptr;
}

template <class X, class Y>
inline typename cast_retty<X, Y *>::ret_type
dyn_cast(Y *Val) {
  return isa<X>(Val) ? cast<X>(Val) : nullptr;
}

// dyn_cast_or_null<X> - Functionally identical to dyn_cast, except that a null
// value is accepted.
//
template <class X, class Y>
inline typename std::enable_if<
    !is_simple_type<Y>::value, typename cast_retty<X, const Y>::ret_type>::type
dyn_cast_or_null(const Y &Val) {
  return (Val && isa<X>(Val)) ? cast<X>(Val) : nullptr;
}

template <class X, class Y>
inline typename std::enable_if<
    !is_simple_type<Y>::value, typename cast_retty<X, Y>::ret_type>::type
dyn_cast_or_null(Y &Val) {
  return (Val && isa<X>(Val)) ? cast<X>(Val) : nullptr;
}

template <class X, class Y>
inline typename cast_retty<X, Y *>::ret_type
dyn_cast_or_null(Y *Val) {
  return (Val && isa<X>(Val)) ? cast<X>(Val) : nullptr;
}

} // namespace rtti

template <class X, class Y>
inline const X * dyn_cast(const Y * Val)
{
  return rtti::dyn_cast_or_null<X>(Val);
}

template <class X>
inline const X * dyn_cast(const void * Val)
{
  return rtti::dyn_cast_or_null<X>(as_object(Val));
}

template <class X, class Y>
inline X * dyn_cast(Y * Val)
{
  return rtti::dyn_cast_or_null<X>(Val);
}

template <class X>
inline X * dyn_cast(void * Val)
{
  return rtti::dyn_cast_or_null<X>(as_object(Val));
}

template <class X, class Y>
inline bool isa(const Y * Val)
{
  return rtti::isa<X>(Val);
}
