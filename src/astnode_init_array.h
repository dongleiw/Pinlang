#pragma once

#include <string>

#include "astnode.h"
#include "define.h"
#include "execute_context.h"
#include "type.h"
#include "variable.h"
#include "verify_context.h"

/*
 */
class AstNodeInitArray : public AstNode {
public:
	AstNodeInitArray(AstNodeType* array_type, std::vector<AstNode*> init_values) {
		m_array_type  = array_type;
		m_init_values = init_values;
	}

	virtual VerifyContextResult Verify(VerifyContext& ctx, VerifyContextParam vr_param) override;
	virtual Variable*			Execute(ExecuteContext& ctx) override;

	virtual AstNode*  DeepClone() override { return DeepCloneT(); }
	AstNodeInitArray* DeepCloneT();

private:
	AstNodeInitArray(){}
private:
	AstNodeType*		  m_array_type;
	std::vector<AstNode*> m_init_values;
};
