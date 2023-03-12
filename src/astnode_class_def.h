#pragma once

#include <string>

#include "astnode.h"
#include "compile_context.h"
#include "define.h"
#include "execute_context.h"
#include "type.h"
#include "variable.h"
#include "verify_context.h"

/*
 */
class AstNodeClassDef : public AstNode {
public:
	AstNodeClassDef(std::string class_name, std::vector<ParserClassField> field_list,
					std::vector<AstNodeComplexFnDef*> method_list, std::vector<AstNode*> subclass_list,
					std::vector<ParserClassImplConstraint> impl_constraint_list) {
		m_class_name		   = class_name;
		m_field_list		   = field_list;
		m_method_list		   = method_list;
		m_subclass_list		   = subclass_list;
		m_impl_constraint_list = impl_constraint_list;
	}

	virtual VerifyContextResult Verify(VerifyContext& ctx, VerifyContextParam vr_param) override;
	virtual Variable*			Execute(ExecuteContext& ctx) override;
	virtual CompileResult		Compile(CompileContext& cctx) override;

	virtual AstNode* DeepClone() override { return DeepCloneT(); }
	AstNodeClassDef* DeepCloneT();

	std::string GetClassName() const { return m_class_name; }

private:
	AstNodeClassDef() {}

private:
	std::string							   m_class_name;
	std::vector<ParserClassField>		   m_field_list;
	std::vector<AstNodeComplexFnDef*>	   m_method_list;
	std::vector<AstNode*>				   m_subclass_list; // 目前忽略掉
	std::vector<ParserClassImplConstraint> m_impl_constraint_list;

	TypeId m_class_tid;
};
