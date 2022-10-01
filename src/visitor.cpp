#include "visitor.h"

#include "astnode.h"
#include "astnode_access_array_element.h"
#include "astnode_access_attr.h"
#include "astnode_blockstmt.h"
#include "astnode_class_def.h"
#include "astnode_complex_fndef.h"
#include "astnode_constraint.h"
#include "astnode_fncall.h"
#include "astnode_generic_instantiate.h"
#include "astnode_identifier.h"
#include "astnode_if.h"
#include "astnode_init_array.h"
#include "astnode_literal.h"
#include "astnode_operator.h"
#include "astnode_return.h"
#include "astnode_type.h"
#include "astnode_vardef.h"
#include "define.h"
#include "log.h"
#include "type.h"
#include "type_constraint.h"
#include "type_fn.h"
#include "type_mgr.h"
#include "type_virtual_gtype.h"
#include "utils.h"
#include "variable.h"
#include <any>

std::any Visitor::visitType_array(PinlangParser::Type_arrayContext* ctx) {
	AstNodeType* r					= new AstNodeType();
	AstNodeType* array_element_type = std::any_cast<AstNodeType*>(ctx->type()->accept(this));
	r->InitWithArray(array_element_type);
	return r;
}
std::any Visitor::visitType(PinlangParser::TypeContext* ctx) {
	AstNodeType* r = new AstNodeType();
	if (ctx->TYPE() != nullptr) {
		r->InitWithType();
	} else if (ctx->Identifier() != nullptr) {
		r->InitWithIdentifier(ctx->Identifier()->getText());
	} else if (ctx->FN() != nullptr) {
		std::vector<ParserParameter> params		 = std::any_cast<std::vector<ParserParameter>>(ctx->parameter_list()->accept(this));
		AstNodeType*				 return_type = nullptr;
		if (ctx->type() != nullptr) {
			return_type = std::any_cast<AstNodeType*>(ctx->type()->accept(this));
		}
		r->InitWithFn(params, return_type);
	} else if (ctx->type_array() != nullptr) {
		return ctx->type_array()->accept(this);
	} else {
		panicf("unknown type");
	}
	return r;
}
std::any Visitor::visitType_list(PinlangParser::Type_listContext* ctx) {
	std::vector<AstNodeType*> type_list;
	for (auto iter : ctx->type()) {
		type_list.push_back(std::any_cast<AstNodeType*>(iter->accept(this)));
	}
	return type_list;
}
std::any Visitor::visitExpr_primary_literal(PinlangParser::Expr_primary_literalContext* ctx) {
	auto literal = ctx->literal();
	if (literal->IntegerLiteral() != nullptr) {
		int value = str_to_int(literal->IntegerLiteral()->getText());
		return (AstNode*)new AstNodeLiteral(value);
	} else if (literal->FloatLiteral() != nullptr) {
		float value = str_to_float(literal->FloatLiteral()->getText());
		return (AstNode*)new AstNodeLiteral(value);
	} else if (literal->BoolLiteral() != nullptr) {
		bool value = str_to_bool(literal->BoolLiteral()->getText());
		return (AstNode*)new AstNodeLiteral(value);
	} else if (literal->StringLiteral() != nullptr) {
		std::string value = literal->StringLiteral()->getText();
		assert(value.size() >= 2 && value.at(0) == '"' && value.at(value.size() - 1) == '"');
		value = value.substr(1, value.size() - 2);
		replace_str(value, "\\n", "\n");
		replace_str(value, "\\t", "\t");
		replace_str(value, "\\\"", "\"");
		return (AstNode*)new AstNodeLiteral(value);
	} else {
		panicf("unknown literal");
	}
	return nullptr;
}
std::any Visitor::visitExpr_primary_identifier(PinlangParser::Expr_primary_identifierContext* ctx) {
	return (AstNode*)new AstNodeIdentifier(ctx->getText());
}
std::any Visitor::visitExpr_primary_parens(PinlangParser::Expr_primary_parensContext* ctx) {
	return ctx->expr()->accept(this);
}
std::any Visitor::visitExpr_relational(PinlangParser::Expr_relationalContext* ctx) {
	AstNode*	left  = std::any_cast<AstNode*>(ctx->expr().at(0)->accept(this));
	AstNode*	right = std::any_cast<AstNode*>(ctx->expr().at(1)->accept(this));
	std::string constraint_name;
	std::string op;
	if (ctx->EQUALS() != NULL) {
		op = "equal";
	} else if (ctx->LESS() != NULL) {
		op = "less";
	} else if (ctx->LESS_OR_EQUALS() != NULL) {
		op = "lessEqual";
	} else if (ctx->GREATER() != NULL) {
		op = "greater";
	} else if (ctx->GREATER_OR_EQUALS() != NULL) {
		op = "greaterEqual";
	} else if (ctx->NOT_EQUALS() != NULL) {
		op = "notEqual";
	} else {
		panicf("unknown op");
	}
	return (AstNode*)new AstNodeOperator(left, constraint_name, op, right);
}
std::any Visitor::visitExpr_muliplicative(PinlangParser::Expr_muliplicativeContext* ctx) {
	AstNode*	left  = std::any_cast<AstNode*>(ctx->expr().at(0)->accept(this));
	AstNode*	right = std::any_cast<AstNode*>(ctx->expr().at(1)->accept(this));
	std::string constraint_name;
	std::string op;
	if (ctx->MUL() != NULL) {
		op = "mul";
	} else if (ctx->DIV() != NULL) {
		op = "div";
	} else if (ctx->MOD() != NULL) {
		op = "mod";
	} else {
		panicf("unknown op");
	}
	return (AstNode*)new AstNodeOperator(left, constraint_name, op, right);
}
std::any Visitor::visitExpr_logical(PinlangParser::Expr_logicalContext* ctx) {
	panicf("not implemented yet\n");
	return nullptr;
}
std::any Visitor::visitExpr_additive(PinlangParser::Expr_additiveContext* ctx) {
	AstNode*	left  = std::any_cast<AstNode*>(ctx->expr().at(0)->accept(this));
	AstNode*	right = std::any_cast<AstNode*>(ctx->expr().at(1)->accept(this));
	std::string constraint_name;
	std::string op;
	if (ctx->ADD() != nullptr) {
		op = "add";
	} else if (ctx->SUB() != nullptr) {
		op = "sub";
	} else {
		panicf("unknown op");
	}
	return (AstNode*)new AstNodeOperator(left, constraint_name, op, right);
}
std::any Visitor::visitExpr_primary_expr(PinlangParser::Expr_primary_exprContext* ctx) {
	return ctx->expr_primary()->accept(this);
}
std::any Visitor::visitExpr_primary_init_array(PinlangParser::Expr_primary_init_arrayContext* ctx) {
	return ctx->expr_init_array()->accept(this);
}
/*
 * 解析一个statement, 返回AstNode*
 */
std::any Visitor::visitStatement(PinlangParser::StatementContext* ctx) {
	if (ctx->expr() != nullptr)
		return ctx->expr()->accept(this);
	else if (ctx->stmt_vardef() != nullptr)
		return ctx->stmt_vardef()->accept(this);
	else if (ctx->stmt_fndef() != nullptr) {
		return ctx->stmt_fndef()->accept(this);
	} else if (ctx->stmt_block() != nullptr) {
		return ctx->stmt_block()->accept(this);
	} else if (ctx->stmt_return() != nullptr) {
		return ctx->stmt_return()->accept(this);
	} else if (ctx->stmt_constraint_def() != nullptr) {
		return ctx->stmt_constraint_def()->accept(this);
	} else if (ctx->stmt_fndef() != nullptr) {
		return ctx->stmt_fndef()->accept(this);
	} else if (ctx->stmt_if() != nullptr) {
		return ctx->stmt_if()->accept(this);
	} else if (ctx->stmt_class_def() != nullptr) {
		return ctx->stmt_class_def()->accept(this);
	} else {
		panicf("bug");
	}
}
std::any Visitor::visitLiteral(PinlangParser::LiteralContext* context) {
	panicf("not implemented yet\n");
	return NULL;
}
std::any Visitor::visitStart(PinlangParser::StartContext* ctx) {
	std::vector<AstNode*> stmts;
	for (auto n : ctx->statement()) {
		std::any ret = n->accept(this);
		if (!ret.has_value()) {
			panicf("visitor returns null");
		} else {
			// log_debug("visitor returns %s", ret.type().name());
		}
		stmts.push_back(std::any_cast<AstNode*>(ret));
	}
	return new AstNodeBlockStmt(stmts);
}
std::any Visitor::visitStmt_vardef(PinlangParser::Stmt_vardefContext* ctx) {
	std::string	 var_name	   = ctx->Identifier()->getText();
	AstNodeType* declared_type = nullptr;
	if (ctx->type() != NULL) {
		declared_type = std::any_cast<AstNodeType*>(ctx->type()->accept(this));
	}
	AstNode* init_expr = NULL;
	if (ctx->expr() != NULL) {
		init_expr = std::any_cast<AstNode*>(ctx->expr()->accept(this));
	}

	bool is_const = ctx->CONST() != nullptr;

	return (AstNode*)new AstNodeVarDef(var_name, declared_type, init_expr, is_const);
}
/*
 * 解析参数
 * @return 返回ParserParameter
 */
std::any Visitor::visitParameter(PinlangParser::ParameterContext* ctx) {
	ParserParameter param;
	if (ctx->Identifier() != nullptr) {
		param.name = ctx->Identifier()->getText();
	}
	param.type = std::any_cast<AstNodeType*>(ctx->type()->accept(this));
	return param;
}
/*
 * 解析一组参数
 * @return std::vector<ParserParameter>
 */
std::any Visitor::visitParameter_list(PinlangParser::Parameter_listContext* ctx) {
	std::vector<ParserParameter> params;
	for (auto iter : ctx->parameter()) {
		params.push_back(std::any_cast<ParserParameter>(iter->accept(this)));
	}
	return params;
}
/*
 * 解析一个函数定义
 * @return
 */
std::any Visitor::visitStmt_simple_fndef(PinlangParser::Stmt_simple_fndefContext* ctx) {
	std::string					 fn_name	 = ctx->Identifier()->getText();
	std::vector<ParserParameter> params		 = std::any_cast<std::vector<ParserParameter>>(ctx->parameter_list()->accept(this));
	AstNodeBlockStmt*			 body		 = dynamic_cast<AstNodeBlockStmt*>(std::any_cast<AstNode*>(ctx->stmt_block()->accept(this)));
	AstNodeType*				 return_type = nullptr;
	if (ctx->type() != nullptr) {
		return_type = std::any_cast<AstNodeType*>(ctx->type()->accept(this));
	}

	AstNodeComplexFnDef::Implement				implement(std::vector<ParserGenericParam>(), params, return_type, body, nullptr);
	std::vector<AstNodeComplexFnDef::Implement> implements;
	implements.push_back(implement);

	return (AstNode*)new AstNodeComplexFnDef(fn_name, implements);

	//return (AstNode*)new AstNodeFnDef(fn_name, params, return_type, body);
}
std::any Visitor::visitStmt_block(PinlangParser::Stmt_blockContext* ctx) {
	std::vector<AstNode*> stmts;
	for (auto iter : ctx->statement()) {
		stmts.push_back(std::any_cast<AstNode*>(iter->accept(this)));
	}
	return (AstNode*)new AstNodeBlockStmt(stmts);
}
std::any Visitor::visitStmt_return(PinlangParser::Stmt_returnContext* ctx) {
	AstNode* expr = nullptr;
	if (ctx->expr() != nullptr) {
		expr = std::any_cast<AstNode*>(ctx->expr()->accept(this));
	}
	return (AstNode*)new AstNodeReturn(expr);
}
/*
 * 解析expr列表
 * 返回 std::vector<AstNode*>
 */
std::any Visitor::visitExpr_list(PinlangParser::Expr_listContext* ctx) {
	std::vector<AstNode*> list;
	for (auto iter : ctx->expr()) {
		list.push_back(std::any_cast<AstNode*>(iter->accept(this)));
	}
	return list;
}
std::any Visitor::visitExpr_primary_fncall(PinlangParser::Expr_primary_fncallContext* ctx) {
	AstNode* fn_expr = std::any_cast<AstNode*>(ctx->expr_primary()->accept(this));

	std::vector<AstNode*> list = std::any_cast<std::vector<AstNode*>>(ctx->expr_list()->accept(this));

	return (AstNode*)new AstNodeFnCall(fn_expr, list);
}

/*
 * 解析函数声明. 返回 ParserFnDeclare
 */
std::any Visitor::visitStmt_fn_declare(PinlangParser::Stmt_fn_declareContext* ctx) {
	std::string					 fnname			   = ctx->Identifier()->getText();
	std::vector<ParserParameter> parser_param_list = std::any_cast<std::vector<ParserParameter>>(ctx->parameter_list()->accept(this));
	AstNodeType*				 return_type	   = nullptr;
	if (ctx->type() != nullptr) {
		return_type = std::any_cast<AstNodeType*>(ctx->type()->accept(this));
	}

	return ParserFnDeclare{
		.fnname		 = fnname,
		.param_list	 = parser_param_list,
		.return_type = return_type,
	};
}
std::any Visitor::visitConstraint_generic_params(PinlangParser::Constraint_generic_paramsContext* ctx) {
	return ctx->identifier_list()->accept(this);
}
std::any Visitor::visitStmt_constraint_def(PinlangParser::Stmt_constraint_defContext* ctx) {
	std::string				 name = ctx->Identifier()->getText();
	std::vector<std::string> generic_params;
	if (ctx->constraint_generic_params() != nullptr) {
		generic_params = std::any_cast<std::vector<std::string>>(ctx->constraint_generic_params()->accept(this));
	}
	std::vector<ParserFnDeclare> rules;
	for (auto iter : ctx->stmt_fn_declare()) {
		ParserFnDeclare fn_declare = std::any_cast<ParserFnDeclare>(iter->accept(this));
		rules.push_back(fn_declare);
	}

	return (AstNode*)new AstNodeConstraint(name, generic_params, rules);
}
/*
 * 解析一个generics参数的约束
 * @return ParserGenericParam
 */
std::any Visitor::visitGeneric_param_constraint(PinlangParser::Generic_param_constraintContext* ctx) {
	ParserGenericParam generic_param;
	generic_param.constraint_name = ctx->Identifier()->getText();
	for (auto iter : ctx->type()) {
		AstNodeType* constraint_type = std::any_cast<AstNodeType*>(iter->accept(this));
		generic_param.constraint_generic_params.push_back(constraint_type);
	}
	return generic_param;
}
/*
 * 解析一个generic参数
 * @return ParserGenericParam
 */
std::any Visitor::visitGeneric_param(PinlangParser::Generic_paramContext* ctx) {
	ParserGenericParam generic_param = std::any_cast<ParserGenericParam>(ctx->generic_param_constraint()->accept(this));
	generic_param.type_name			 = ctx->Identifier()->getText();
	return generic_param;
}
/*
 * 解析一个泛型函数
 */
std::any Visitor::visitStmt_generic_fndef(PinlangParser::Stmt_generic_fndefContext* ctx) {
	std::string						fn_name = ctx->Identifier()->getText();
	std::vector<ParserGenericParam> generic_params;
	for (auto iter : ctx->generic_param()) {
		ParserGenericParam generic_param = std::any_cast<ParserGenericParam>(iter->accept(this));
		generic_params.push_back(generic_param);
	}

	std::vector<ParserParameter> params = std::any_cast<std::vector<ParserParameter>>(ctx->parameter_list()->accept(this));

	AstNodeType* return_type = nullptr;
	if (ctx->type() != nullptr) {
		return_type = std::any_cast<AstNodeType*>(ctx->type()->accept(this));
	}

	AstNodeBlockStmt* body = dynamic_cast<AstNodeBlockStmt*>(std::any_cast<AstNode*>(ctx->stmt_block()->accept(this)));

	AstNodeComplexFnDef::Implement				implement(generic_params, params, return_type, body, nullptr);
	std::vector<AstNodeComplexFnDef::Implement> implements;
	implements.push_back(implement);
	return (AstNode*)new AstNodeComplexFnDef(fn_name, implements);
	//return (AstNode*)new AstNodeGenericFnDef(fn_name, generic_params, params, return_type, body);
}
// return std::vector<std::string>
std::any Visitor::visitIdentifier_list(PinlangParser::Identifier_listContext* ctx) {
	std::vector<std::string> ids;
	for (auto iter : ctx->Identifier()) {
		ids.push_back(iter->getText());
	}
	return ids;
}
//std::any Visitor::visitExpr_primary_gparam(PinlangParser::Expr_primary_gparamContext* ctx) {
//	std::string				  id		= ctx->Identifier()->getText();
//	std::vector<AstNodeType*> type_list = std::any_cast<std::vector<AstNodeType*>>(ctx->type_list()->accept(this));
//	return (AstNode*)new AstNodeGenericInstantiate(id, type_list);
//}
std::any Visitor::visitExpr_primary_access_attr(PinlangParser::Expr_primary_access_attrContext* ctx) {
	AstNode*	expr	  = std::any_cast<AstNode*>(ctx->expr_primary()->accept(this));
	std::string attr_name = ctx->Identifier()->getText();
	return (AstNode*)new AstNodeAccessAttr(expr, attr_name);
}
std::any Visitor::visitStmt_if(PinlangParser::Stmt_ifContext* ctx) {
	std::vector<AstNode*> cond_exprs;
	std::vector<AstNode*> cond_blocks;
	AstNode*			  else_cond_block = nullptr;
	for (size_t i = 0; i < ctx->expr().size(); i++) {
		cond_exprs.push_back(std::any_cast<AstNode*>(ctx->expr().at(i)->accept(this)));
		cond_blocks.push_back(std::any_cast<AstNode*>(ctx->stmt_block().at(i)->accept(this)));
	}
	if (ctx->stmt_block().size() == ctx->expr().size()) {
	} else if (ctx->stmt_block().size() == ctx->expr().size() + 1) {
		else_cond_block = std::any_cast<AstNode*>(ctx->stmt_block().at(ctx->expr().size())->accept(this));
	} else {
		panicf("bug");
	}
	return (AstNode*)new AstNodeIf(cond_exprs, cond_blocks, else_cond_block);
}
std::any Visitor::visitStmt_complex_fndef_implement(PinlangParser::Stmt_complex_fndef_implementContext* ctx) {
	std::vector<ParserGenericParam> generic_params;
	for (auto iter : ctx->generic_param()) {
		ParserGenericParam generic_param = std::any_cast<ParserGenericParam>(iter->accept(this));
		generic_params.push_back(generic_param);
	}
	std::vector<ParserParameter> params		 = std::any_cast<std::vector<ParserParameter>>(ctx->parameter_list()->accept(this));
	AstNodeType*				 return_type = nullptr;
	if (ctx->type() != nullptr) {
		return_type = std::any_cast<AstNodeType*>(ctx->type()->accept(this));
	}
	AstNodeBlockStmt* body = dynamic_cast<AstNodeBlockStmt*>(std::any_cast<AstNode*>(ctx->stmt_block()->accept(this)));

	AstNodeComplexFnDef::Implement implement(generic_params, params, return_type, body, nullptr);

	return implement;
}
std::any Visitor::visitStmt_complex_fndef(PinlangParser::Stmt_complex_fndefContext* ctx) {
	std::string									fn_name = ctx->Identifier()->getText();
	std::vector<AstNodeComplexFnDef::Implement> implements;
	for (auto iter : ctx->stmt_complex_fndef_implement()) {
		AstNodeComplexFnDef::Implement implement = std::any_cast<AstNodeComplexFnDef::Implement>(iter->accept(this));
		implements.push_back(implement);
	}
	return (AstNode*)new AstNodeComplexFnDef(fn_name, implements);
}
std::any Visitor::visitExpr_init_array(PinlangParser::Expr_init_arrayContext* ctx) {
	AstNodeType*		  astnode_type = std::any_cast<AstNodeType*>(ctx->type_array()->accept(this));
	std::vector<AstNode*> list		   = std::any_cast<std::vector<AstNode*>>(ctx->expr_list()->accept(this));
	return (AstNode*)new AstNodeInitArray(astnode_type, list);
}
std::any Visitor::visitExpr_primary_access_array_element(PinlangParser::Expr_primary_access_array_elementContext* ctx) {
	AstNode* array_expr = std::any_cast<AstNode*>(ctx->expr_primary()->accept(this));
	AstNode* index_expr = std::any_cast<AstNode*>(ctx->expr()->accept(this));

	return (AstNode*)new AstNodeAccessArrayElement(array_expr, index_expr);
}
std::any Visitor::visitStmt_class_def(PinlangParser::Stmt_class_defContext* ctx) {
	std::string					  class_name = ctx->Identifier().at(0)->getText();
	std::vector<ParserClassField> class_field_list;
	{
		auto ids   = ctx->Identifier();
		auto types = ctx->type();
		assert(ids.size() == types.size() + 1);
		for (size_t i = 0; i < types.size(); i++) {
			ParserClassField field{
				.name = ids.at(i+1)->getText(),
				.type = std::any_cast<AstNodeType*>(types.at(i)->accept(this)),
			};
			class_field_list.push_back(field);
		}
	}

	std::vector<AstNodeComplexFnDef*> class_fn_list;
	for (auto iter : ctx->stmt_fndef()) {
		AstNode*			 astnode = std::any_cast<AstNode*>(iter->accept(this));
		AstNodeComplexFnDef* n		 = dynamic_cast<AstNodeComplexFnDef*>(astnode);
		assert(n != nullptr);
		class_fn_list.push_back(n);
	}

	std::vector<AstNode*> class_subclass_def_list;
	for (auto iter : ctx->stmt_class_def()) {
		class_subclass_def_list.push_back(std::any_cast<AstNode*>(iter->accept(this)));
	}

	return (AstNode*)new AstNodeClassDef(class_name, class_field_list, class_fn_list, class_subclass_def_list);
}
std::any Visitor::visitStmt_fndef(PinlangParser::Stmt_fndefContext* ctx) {
	if (ctx->stmt_simple_fndef() != nullptr) {
		return ctx->stmt_simple_fndef()->accept(this);
	} else if (ctx->stmt_generic_fndef() != nullptr) {
		return ctx->stmt_generic_fndef()->accept(this);
	} else if (ctx->stmt_complex_fndef() != nullptr) {
		return ctx->stmt_complex_fndef()->accept(this);
	} else {
		panicf("bug");
	}
}
