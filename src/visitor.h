#pragma once

#include "PinlangBaseVisitor.h"

class Visitor : public PinlangBaseVisitor {
  public:
	virtual std::any visitType(PinlangParser::TypeContext* context) override;
	virtual std::any visitPrimary_expr_literal(PinlangParser::Primary_expr_literalContext* context) override;
	virtual std::any visitPrimary_expr_identifier(PinlangParser::Primary_expr_identifierContext* context) override;
	virtual std::any visitPrimary_expr_parens(PinlangParser::Primary_expr_parensContext* context) override;
	virtual std::any visitExpr_relational(PinlangParser::Expr_relationalContext* context) override;
	virtual std::any visitExpr_muliplicative(PinlangParser::Expr_muliplicativeContext* context) override;
	virtual std::any visitExpr_logical(PinlangParser::Expr_logicalContext* context) override;
	virtual std::any visitExpr_additive(PinlangParser::Expr_additiveContext* context) override;
	virtual std::any visitExpr_primary_expr(PinlangParser::Expr_primary_exprContext* context) override;
	virtual std::any visitStatement(PinlangParser::StatementContext* context) override;
	virtual std::any visitLiteral(PinlangParser::LiteralContext* context) override;
	virtual std::any visitStart(PinlangParser::StartContext* context) override;
	virtual std::any visitStmt_vardef(PinlangParser::Stmt_vardefContext* ctx) override;
};
