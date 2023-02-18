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

	virtual std::any visitType_value(PinlangParser::Type_valueContext* ctx) override;
	virtual std::any visitType_integer(PinlangParser::Type_integerContext* ctx) override;
	virtual std::any visitType_float(PinlangParser::Type_floatContext* ctx) override;
	virtual std::any visitType_bool(PinlangParser::Type_boolContext* ctx) override;
	virtual std::any visitType_fn(PinlangParser::Type_fnContext* ctx) override;
	virtual std::any visitType_array_static_size(PinlangParser::Type_array_static_sizeContext* ctx) override;
	virtual std::any visitType_pointer(PinlangParser::Type_pointerContext* ctx) override;

	virtual std::any visitType_reference(PinlangParser::Type_referenceContext* ctx) override;
	virtual std::any visitType_array_dynamic_size(PinlangParser::Type_array_dynamic_sizeContext* ctx) override;
	virtual std::any visitType_str(PinlangParser::Type_strContext* ctx) override;
	virtual std::any visitType_tuple(PinlangParser::Type_tupleContext* ctx) override;

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
	// virtual std::any visitExpr_primary_gparam(PinlangParser::Expr_primary_gparamContext* ctx) override;
	virtual std::any visitExpr_primary_access_array_element(PinlangParser::Expr_primary_access_array_elementContext* ctx) override;
	virtual std::any visitExpr_primary_access_attr(PinlangParser::Expr_primary_access_attrContext* ctx) override;
	virtual std::any visitExpr_primary_init(PinlangParser::Expr_primary_initContext* ctx) override;
	virtual std::any visitExpr_dereference(PinlangParser::Expr_dereferenceContext* ctx) override;
	virtual std::any visitExpr_reference(PinlangParser::Expr_referenceContext* ctx) override;

	virtual std::any visitExpr_init_element(PinlangParser::Expr_init_elementContext* context) override;
	virtual std::any visitExpr_init_body(PinlangParser::Expr_init_bodyContext* context) override;
	virtual std::any visitExpr_init(PinlangParser::Expr_initContext* context) override;

	virtual std::any visitStatement(PinlangParser::StatementContext* context) override;
	virtual std::any visitLiteral(PinlangParser::LiteralContext* context) override;
	virtual std::any visitStart(PinlangParser::StartContext* context) override;
	virtual std::any visitStmt_vardef(PinlangParser::Stmt_vardefContext* ctx) override;
	virtual std::any visitParameter(PinlangParser::ParameterContext* context) override;
	virtual std::any visitParameter_list(PinlangParser::Parameter_listContext* context) override;
	virtual std::any visitStmt_block(PinlangParser::Stmt_blockContext* context) override;
	virtual std::any visitStmt_return(PinlangParser::Stmt_returnContext* context) override;
	virtual std::any visitExpr_list(PinlangParser::Expr_listContext* ctx) override;
	virtual std::any visitExpr_list_optional(PinlangParser::Expr_list_optionalContext* ctx) override;
	virtual std::any visitStmt_fn_declare(PinlangParser::Stmt_fn_declareContext* ctx) override;
	virtual std::any visitStmt_if(PinlangParser::Stmt_ifContext* ctx) override;
	virtual std::any visitStmt_assignment(PinlangParser::Stmt_assignmentContext* ctx) override;
	virtual std::any visitStmt_break(PinlangParser::Stmt_breakContext* ctx) override;

	virtual std::any visitStmt_for_init(PinlangParser::Stmt_for_initContext* ctx) override;
	virtual std::any visitStmt_for_cond(PinlangParser::Stmt_for_condContext* ctx) override;
	virtual std::any visitStmt_for_loop(PinlangParser::Stmt_for_loopContext* ctx) override;
	virtual std::any visitStmt_for(PinlangParser::Stmt_forContext* ctx) override;
	virtual std::any visitStmt_while(PinlangParser::Stmt_whileContext* ctx) override;

	virtual std::any visitStmt_fndef(PinlangParser::Stmt_fndefContext* ctx) override;
	virtual std::any visitStmt_simple_fndef(PinlangParser::Stmt_simple_fndefContext* ctx) override;
	virtual std::any visitStmt_generic_fndef(PinlangParser::Stmt_generic_fndefContext* ctx) override;
	virtual std::any visitStmt_complex_fndef_implement(PinlangParser::Stmt_complex_fndef_implementContext* ctx) override;
	virtual std::any visitStmt_complex_fndef(PinlangParser::Stmt_complex_fndefContext* ctx) override;

	virtual std::any visitStmt_class_def_impl_constraint(PinlangParser::Stmt_class_def_impl_constraintContext* ctx) override;
	virtual std::any visitStmt_class_def(PinlangParser::Stmt_class_defContext* ctx) override;

	virtual std::any visitConstraint_generic_params(PinlangParser::Constraint_generic_paramsContext* ctx) override;
	virtual std::any visitStmt_constraint_def(PinlangParser::Stmt_constraint_defContext* ctx) override;

	virtual std::any visitGeneric_param(PinlangParser::Generic_paramContext* ctx) override;
	virtual std::any visitGeneric_param_constraint(PinlangParser::Generic_param_constraintContext* ctx) override;
	virtual std::any visitIdentifier_list(PinlangParser::Identifier_listContext* ctx) override;
};
