#include "type_fn.h"
#include "astnode_complex_fndef.h"
#include "astnode_constraint.h"
#include "define.h"
#include "function_obj.h"
#include "log.h"
#include "type.h"
#include "type_mgr.h"
#include "utils.h"

TypeInfoFn::TypeInfoFn(std::vector<TypeId> params, TypeId return_tid) {
	m_params	 = params;
	m_return_tid = return_tid;
	set_name();
	//set_uniq_fn_name_suffix();
	m_typegroup_id	 = TYPE_GROUP_ID_FUNCTION;
	m_mem_size		 = sizeof(FunctionObj);
	m_mem_align_size = 8;
}

std::string TypeInfoFn::GetUniqFnName(TypeId obj_tid, std::string fnname, std::vector<TypeId> concrete_generic_params, std::vector<TypeId> params_tid, TypeId return_tid) {
	std::string fnid;
	if (obj_tid != TYPE_ID_NONE) {
		fnid = sprintf_to_stdstr("%d:%s::", obj_tid, GET_TYPENAME_C(obj_tid));
	}
	fnid += fnname + "[";
	for (size_t i = 0; i < concrete_generic_params.size(); i++) {
		fnid = fnid + sprintf_to_stdstr("%d:%s", concrete_generic_params.at(i), GET_TYPENAME_C(concrete_generic_params.at(i)));
		if (i + 1 != concrete_generic_params.size()) {
			fnid += ",";
		}
	}
	fnid += "](";
	for (size_t i = 0; i < params_tid.size(); i++) {
		fnid = fnid + sprintf_to_stdstr("%d:%s", params_tid.at(i), GET_TYPENAME_C(params_tid.at(i)));
		if (i + 1 != params_tid.size()) {
			fnid += ",";
		}
	}
	fnid += ")";
	if (return_tid != TYPE_ID_NONE) {
		fnid += sprintf_to_stdstr("%d:%s", return_tid, GET_TYPENAME_C(return_tid));
	}
	return fnid;
}
std::string TypeInfoFn::GetUniqFnName(TypeId obj_tid, std::string fnname, std::vector<TypeId> params_tid, TypeId return_tid) {
	return GetUniqFnName(obj_tid, fnname, std::vector<TypeId>(), params_tid, return_tid);
}
void TypeInfoFn::set_name() {
	m_name = "fn" + GetUniqFnName(TYPE_ID_NONE, "", m_params, GetReturnTypeId());
}
bool TypeInfoFn::VerifyArgsType(std::vector<TypeId> args_type) {
	if (args_type.size() != m_params.size()) {
		return false;
	}

	for (size_t i = 0; i < m_params.size(); i++) {
		TypeId expect_tid = m_params.at(i);
		TypeId give_tid	  = args_type.at(i);
		if (expect_tid != give_tid) {
			log_error("type of arg %lu not match: expect[%s] give[%s]", i, GET_TYPENAME_C(expect_tid), GET_TYPENAME_C(give_tid));
			return false;
		}
	}
	return true;
}
bool TypeInfoFn::IsArgsTypeEqual(const TypeInfoFn& another) const {
	if (m_params.size() != another.m_params.size()) {
		return false;
	}
	for (size_t i = 0; i < m_params.size(); i++) {
		if (m_params.at(i) != another.m_params.at(i)) {
			return false;
		}
	}
	return true;
}
bool TypeInfoFn::IsArgsTypeEqual(std::vector<TypeId> args_tid) const {
	if (m_params.size() != args_tid.size()) {
		return false;
	}
	for (size_t i = 0; i < m_params.size(); i++) {
		if (m_params.at(i) != args_tid.at(i)) {
			return false;
		}
	}
	return true;
}
std::vector<TypeId> TypeInfoFn::GetParmsTid() const {
	std::vector<TypeId> params_tid;
	for (auto iter : m_params) {
		params_tid.push_back(iter);
	}
	return params_tid;
}
void TypeInfoFn::InitBuiltinMethods(VerifyContext& ctx) {
	ctx.PushStack();
	ctx.GetCurStack()->EnterBlock(new VariableTable());
	// 手动实现Fn约束
	{
		std::vector<AstNodeComplexFnDef*> fns;
		AstNodeConstraint*				  constraint	 = ctx.GetCurStack()->GetVariable("Fn")->GetValueConstraint();
		TypeId							  constraint_tid = constraint->Instantiate(ctx, std::vector<TypeId>{});
		AddConstraint(constraint_tid, fns);
	}
	ctx.PopSTack();
}
