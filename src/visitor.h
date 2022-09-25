#pragma once

#include "PinlangBaseVisitor.h"

class Visitor : public PinlangBaseVisitor {
public:
	/*
	 * 解析数组类型
	 */
	virtual std::any visitType_array(PinlangParser::Type_arrayContext* ctx) override;
	/*
	 * 解析类型. 返回AstNodeType*
	 */
	virtual std::any visitType(PinlangParser::TypeContext* ctx) override;
	/*
	 * 解析类型列表: int, float, str
	 * return: std::vector<AstNodeType*>
	 */
	virtual std::any visitType_list(PinlangParser::Type_listContext* ctx) override;

	virtual std::any visitExpr_primary_expr(PinlangParser::Expr_primary_exprContext* context) override;
	virtual std::any visitExpr_relational(PinlangParser::Expr_relationalContext* context) override;
	virtual std::any visitExpr_muliplicative(PinlangParser::Expr_muliplicativeContext* context) override;
	virtual std::any visitExpr_logical(PinlangParser::Expr_logicalContext* context) override;
	virtual std::any visitExpr_additive(PinlangParser::Expr_additiveContext* context) override;

	virtual std::any visitExpr_primary_literal(PinlangParser::Expr_primary_literalContext* context) override;
	virtual std::any visitExpr_primary_identifier(PinlangParser::Expr_primary_identifierContext* context) override;
	virtual std::any visitExpr_primary_parens(PinlangParser::Expr_primary_parensContext* context) override;
	virtual std::any visitExpr_primary_fncall(PinlangParser::Expr_primary_fncallContext* ctx) override;
	virtual std::any visitExpr_primary_gparam(PinlangParser::Expr_primary_gparamContext* ctx) override;
	virtual std::any visitExpr_primary_access_attr(PinlangParser::Expr_primary_access_attrContext* ctx) override;
	virtual std::any visitExpr_primary_init_array(PinlangParser::Expr_primary_init_arrayContext* ctx) override;

	virtual std::any visitExpr_init_array(PinlangParser::Expr_init_arrayContext* ctx) override;

	virtual std::any visitStatement(PinlangParser::StatementContext* context) override;
	virtual std::any visitLiteral(PinlangParser::LiteralContext* context) override;
	virtual std::any visitStart(PinlangParser::StartContext* context) override;
	virtual std::any visitStmt_vardef(PinlangParser::Stmt_vardefContext* ctx) override;
	virtual std::any visitParameter(PinlangParser::ParameterContext* context) override;
	virtual std::any visitParameter_list(PinlangParser::Parameter_listContext* context) override;
	virtual std::any visitStmt_fndef(PinlangParser::Stmt_fndefContext* context) override;
	virtual std::any visitStmt_block(PinlangParser::Stmt_blockContext* context) override;
	virtual std::any visitStmt_return(PinlangParser::Stmt_returnContext* context) override;
	virtual std::any visitExpr_list(PinlangParser::Expr_listContext* ctx) override;
	virtual std::any visitStmt_fn_declare(PinlangParser::Stmt_fn_declareContext* ctx) override;
	virtual std::any visitStmt_if(PinlangParser::Stmt_ifContext* ctx) override;
	virtual std::any visitStmt_complex_fndef_implement(PinlangParser::Stmt_complex_fndef_implementContext* ctx) override;
	virtual std::any visitStmt_complex_fndef(PinlangParser::Stmt_complex_fndefContext* ctx) override;

	virtual std::any visitConstraint_generic_params(PinlangParser::Constraint_generic_paramsContext* ctx) override;
	virtual std::any visitStmt_constraint_def(PinlangParser::Stmt_constraint_defContext* ctx) override;

	virtual std::any visitStmt_generic_fndef(PinlangParser::Stmt_generic_fndefContext* ctx) override;
	virtual std::any visitGeneric_param(PinlangParser::Generic_paramContext* ctx) override;
	virtual std::any visitGeneric_param_constraint(PinlangParser::Generic_param_constraintContext* ctx) override;
	virtual std::any visitIdentifier_list(PinlangParser::Identifier_listContext* ctx) override;
};
