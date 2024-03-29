#pragma once
#include "builtin_fn.h"
#include "compile_context.h"
#include "define.h"
#include "instruction.h"

#include <llvm-12/llvm/IR/Function.h>
#include <string>
#include <vector>

class AstNodeBlockStmt;
class VerifyContext;

enum FnKind {
	FN_KIND_USERDEF,
	FN_KIND_BUILTIN,
	FN_KIND_DYNAMIC,
};
struct FnAddr {
	FnKind fn_kind;
	int	   idx;

	FnAddr() : idx(-1) {
	}
};

struct DynamicFnInfo;

// 指向内置函数的执行函数的指针
typedef Variable* (*DynamicFnExecuteCallback)(DynamicFnInfo& dynamic_fn, ExecuteContext& ctx, Variable* thisobj, std::vector<Variable*> args);

class FnParam {
private:
	std::string m_param_name;
	FnParamAttr m_attr;

public:
	FnParam(std::string param_name) : m_param_name(param_name), m_attr(FN_PARAM_ATTR_NONE) {
	}
	FnParam(std::string param_name, FnParamAttr attr) : m_param_name(param_name), m_attr(attr) {
	}
	bool		IsSelfParam() const { return m_attr & FN_PARAM_ATTR_PTR_SELF; }
	std::string GetParamName() const { return m_param_name; }
};

// 用户定义函数
class FnInfo {
public:
	FnInfo();
	FnInfo(std::string fnname, TypeId fn_tid, TypeId obj_tid, std::vector<ConcreteGParam> gparams, std::vector<FnParam> params, FnAttr fn_attr, BuiltinFnCompileCallback compile_cb);
	FnInfo(std::string fnname, TypeId fn_tid, TypeId obj_tid, std::vector<ConcreteGParam> gparams, std::vector<FnParam> params, FnAttr fn_attr, AstNodeBlockStmt* body, bool is_nested);

	bool						IsMethod() const;
	bool						HasSelfParam() const;
	llvm::Function*				GetLLVMIRFn() const;
	bool						IsConstructor() const;
	std::string					GetFnName() const;
	std::string					GetFnId() const;
	TypeId						GetObjTid() const;
	TypeId						GetParamType_no_self(size_t idx) const;
	std::vector<ConcreteGParam> GetGParams() const;
	std::vector<FnParam>		GetParams() const;
	size_t						GetParamNum() const;
	size_t						GetParamNum_no_self() const;
	std::string					GetParamName(size_t i) const;
	TypeId						GetFnTid() const;
	bool						IsNested() const;
	AstNodeBlockStmt*			GetBody() const;
	bool						IsBuiltin() const;
	BuiltinFnCompileCallback	GetCompileCB() const;

	void SetLLVMIRFn(llvm::Function* llvm_ir_fn);
	void SetFnId(std::string fn_id);

private:
	std::string					m_fn_name;
	std::string					m_fn_id;
	TypeId						m_fn_tid;
	TypeId						m_obj_tid;
	std::vector<ConcreteGParam> m_gparams;
	std::vector<FnParam>		m_params;
	AstNodeBlockStmt*			m_body;
	FnAttr						m_fn_attr;
	BuiltinFnCompileCallback	m_compile_cb;
	llvm::Function*				m_llvm_ir_fn;
	bool						m_is_nested;
};

// 运行时动态导入函数
struct DynamicFnInfo {
	TypeId					 fn_tid;
	int						 dynlib_instance_id;
	void*					 dynlib_fn;
	DynamicFnExecuteCallback execute_cb;
};

/*
 * 保存所有函数(包括方法)
 * TODO: 改成唯一fnid
 */
class FnTable {
public:
public:
	FnTable() {}

	TypeId GetFnTypeId(std::string fnid) const;
	TypeId GetFnReturnTypeId(std::string fnid) const;
	bool   IsMethod(std::string fnid) const;
	bool   IsConstructor(std::string fnid) const;

	//std::string AddUserDefineFn(VerifyContext& ctx, std::string fnname, TypeId fn_tid, TypeId obj_tid, std::vector<ConcreteGParam> gparams, std::vector<std::string> params_name, AstNodeBlockStmt* body, FnAttr fn_attr, bool is_nested);
	std::string AddUserDefineFn(VerifyContext& ctx, FnInfo fn_info);
	FnAddr		AddDynamicFn(TypeId fn_tid, int dynlib_instance_id, void* dynlib_fn, DynamicFnExecuteCallback cb);

	const FnInfo* GetFnInfo(std::string fnid) const;

public:
	void Compile(CompileContext& cctx);

private:
	void		compile_user_define_fn(CompileContext& cctx);
	void		compile_builtin_fn(CompileContext& cctx);
	std::string generate_fnid(std::string user_def_fnname, TypeId obj_tid, std::vector<ConcreteGParam> concrete_generic_params, TypeId fn_tid, bool is_nested);

private:
	std::vector<DynamicFnInfo> m_dynamic_fn_table;

	std::map<std::string, FnInfo> m_fn_table;

	std::map<std::string, int> m_nested_fnid_seed;
};
