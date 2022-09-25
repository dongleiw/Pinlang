#pragma once

#include <string>

#include "astnode.h"
#include "execute_context.h"
#include "type.h"
#include "variable.h"
#include "verify_context.h"

/*
 */
class AstNodeCompileConst : public AstNode {
public:
	AstNodeCompileConst(Variable* const_value) {
		m_result_typeid = const_value->GetTypeId();
		m_const_value	= const_value;
	}

	virtual VerifyContextResult Verify(VerifyContext& ctx, VerifyContextParam vr_param) override;
	virtual Variable*			Execute(ExecuteContext& ctx) override;

	virtual AstNode*	 DeepClone() override { return DeepCloneT(); }
	AstNodeCompileConst* DeepCloneT();

private:
	Variable* m_const_value;
};
