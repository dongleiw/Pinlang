#include "type_class.h"

#include <assert.h>
#include <llvm-12/llvm/IR/Function.h>
#include <map>
#include <vector>

#include "astnode_complex_fndef.h"
#include "astnode_constraint.h"
#include "astnode_type.h"
#include "builtin_fn.h"
#include "compile_context.h"
#include "define.h"
#include "execute_context.h"
#include "type.h"
#include "type_fn.h"
#include "type_mgr.h"
#include "variable.h"
#include "verify_context.h"

TypeInfoClass::TypeInfoClass(std::string class_name) {
	m_original_name = class_name;
	m_typegroup_id	= TYPE_GROUP_ID_CLASS;
	m_ir_type		= nullptr;
}
void TypeInfoClass::InitBuiltinMethods(VerifyContext& ctx) {
}
llvm::Type* TypeInfoClass::GetLLVMIRType(CompileContext& cctx) {
	if (m_ir_type == nullptr) {
		std::vector<llvm::Type*> ir_type_fields;
		for (auto& f : m_field_list) {
			TypeInfo* ti_field = g_typemgr.GetTypeInfo(f.tid);
			ir_type_fields.push_back(ti_field->GetLLVMIRType(cctx));
		}
		if (ir_type_fields.empty()) {
			m_ir_type = llvm::StructType::get(IRC);
			m_ir_type->setName(m_name);
		} else {
			m_ir_type = llvm::StructType::create(ir_type_fields, m_name);
		}
	}
	return m_ir_type;
}
void TypeInfoClass::ConstructFields(CompileContext& cctx, llvm::Value* obj) {
	for (size_t i = 0; i < m_field_list.size(); i++) {
		const Field& field		 = m_field_list.at(i);
		TypeInfo*	 ti			 = g_typemgr.GetTypeInfo(field.tid);
		llvm::Value* field_value = IRB.CreateConstGEP2_32(this->GetLLVMIRType(cctx), obj, 0, i);
		ti->ConstructDefault(cctx, field_value);
	}
}
void TypeInfoClass::ConstructDefault(CompileContext& cctx, llvm::Value* obj) {
	if (m_default_constructor_fnid.empty()) {
		panicf("no default-constructor exists");
	}
	llvm::Function* constructor = IRM.getFunction(m_default_constructor_fnid);
	IRB.CreateCall(constructor, std::vector<llvm::Value*>{obj});
}
void TypeInfoClass::AddDefaultConstructor(VerifyContext& ctx) {
	ctx.PushStack();
	ctx.GetCurStack()->EnterBlock(new VariableTable());

	std::vector<AstNodeComplexFnDef::Implement> implements;
	{
		std::vector<ParserGenericParam> gparams;
		std::vector<ParserParameter>	params;
		AstNodeType*					self_param_type = new AstNodeType();
		self_param_type->InitWithIdentifier("self");
		AstNodeType* ptr_self_param_type = new AstNodeType();
		ptr_self_param_type->InitWithPointer(self_param_type);
		params.push_back(ParserParameter{
			.name = "",
			.type = ptr_self_param_type,
		});
		implements.push_back(AstNodeComplexFnDef::Implement(gparams, params, nullptr, new AstNodeBlockStmt()));
	}
	AstNodeComplexFnDef* astnode_complex_fndef = new AstNodeComplexFnDef(m_original_name, implements, FnAttr::FN_ATTR_CONSTRUCTOR);
	astnode_complex_fndef->SetObjTypeId(m_typeid);
	astnode_complex_fndef->Verify(ctx, VerifyContextParam());

	for (auto& constraint : m_constraints) {
		if (constraint.constraint_instance.constraint_instance_name.empty()) {
			constraint.methods.push_back(Method{
				.method_name = astnode_complex_fndef->GetName(),
				.method_node = astnode_complex_fndef,
			});
		}
	}

	ctx.PopStack();
}
size_t TypeInfoClass::GetFieldIndex(std::string field_name) const {
	for (size_t i = 0; i < m_field_list.size(); i++) {
		if (m_field_list.at(i).name == field_name) {
			return i;
		}
	}
	panicf("bug");
}
TypeId TypeInfoClass::GetFieldType(std::string field_name) const {
	for (auto iter : m_field_list) {
		if (iter.name == field_name) {
			return iter.tid;
		}
	}
	panicf("bug");
}
void TypeInfoClass::SetFields(std::vector<std::pair<std::string, TypeId>> fields) {
	assert(m_field_list.empty());
	for (auto iter : fields) {
		m_field_list.push_back(Field{
			.name = iter.first,
			.tid  = iter.second,
		});
	}
}
bool TypeInfoClass::HasField(std::string field_name) const {
	for (auto iter : m_field_list) {
		if (iter.name == field_name) {
			return true;
		}
	}
	return false;
}
