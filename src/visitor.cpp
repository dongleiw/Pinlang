#include "visitor.h"

#include "astnode.h"
#include "astnode_access_array_element.h"
#include "astnode_access_attr.h"
#include "astnode_assignment.h"
#include "astnode_blockstmt.h"
#include "astnode_break.h"
#include "astnode_class_def.h"
#include "astnode_complex_fndef.h"
#include "astnode_constraint.h"
#include "astnode_continue.h"
#include "astnode_dereference.h"
#include "astnode_fncall.h"
#include "astnode_for.h"
#include "astnode_generic_instantiate.h"
#include "astnode_identifier.h"
#include "astnode_if.h"
#include "astnode_init.h"
#include "astnode_literal_bool.h"
#include "astnode_literal_float.h"
#include "astnode_literal_integer.h"
#include "astnode_literal_nullptr.h"
#include "astnode_literal_str.h"
#include "astnode_logical_operator.h"
#include "astnode_new.h"
#include "astnode_operator.h"
#include "astnode_reference.h"
#include "astnode_return.h"
#include "astnode_tuple.h"
#include "astnode_type.h"
#include "astnode_type_cast.h"
#include "astnode_vardef.h"
#include "define.h"
#include "log.h"
#include "source_info.h"
#include "type.h"
#include "type_constraint.h"
#include "type_fn.h"
#include "type_mgr.h"
#include "type_virtual_gtype.h"
#include "utils.h"
#include "variable.h"
#include <any>

std::any Visitor::visitType_array(PinlangParser::Type_arrayContext* ctx) {
	AstNodeType* r			  = new AstNodeType();
	AstNodeType* element_type = std::any_cast<AstNodeType*>(ctx->type()->accept(this));
	AstNode*	 size_expr	  = std::any_cast<AstNode*>(ctx->expr()->accept(this));
	r->InitWithArray(element_type, size_expr);
	return r;
}
std::any Visitor::visitType_array_size_optional(PinlangParser::Type_array_size_optionalContext* ctx) {
	AstNodeType* r			  = new AstNodeType();
	AstNodeType* element_type = std::any_cast<AstNodeType*>(ctx->type()->accept(this));
	AstNode*	 size_expr	  = nullptr;
	if (ctx->expr() != nullptr) {
		size_expr = std::any_cast<AstNode*>(ctx->expr()->accept(this));
	}
	r->InitWithArray(element_type, size_expr);
	return r;
}
std::any Visitor::visitType_tuple(PinlangParser::Type_tupleContext* ctx) {
	AstNodeType*			  r					  = new AstNodeType();
	std::vector<AstNodeType*> tuple_element_types = std::any_cast<std::vector<AstNodeType*>>(ctx->type_list()->accept(this));
	r->InitWithTuple(tuple_element_types);
	return r;
}
std::any Visitor::visitType_fn(PinlangParser::Type_fnContext* ctx) {
	AstNodeType*				 r			 = new AstNodeType();
	std::vector<ParserParameter> params		 = std::any_cast<std::vector<ParserParameter>>(ctx->parameter_list()->accept(this));
	AstNodeType*				 return_type = nullptr;
	if (ctx->type() != nullptr) {
		return_type = std::any_cast<AstNodeType*>(ctx->type()->accept(this));
	}
	r->InitWithFn(params, return_type);
	return r;
}
std::any Visitor::visitType_integer(PinlangParser::Type_integerContext* ctx) {
	AstNodeType* r = new AstNodeType();
	if (ctx->INT_I8() != nullptr) {
		r->InitWithTargetTypeId(TYPE_ID_INT8);
	} else if (ctx->INT_I16() != nullptr) {
		r->InitWithTargetTypeId(TYPE_ID_INT16);
	} else if (ctx->INT_I32() != nullptr) {
		r->InitWithTargetTypeId(TYPE_ID_INT32);
	} else if (ctx->INT_I64() != nullptr) {
		r->InitWithTargetTypeId(TYPE_ID_INT64);
	} else if (ctx->INT_U8() != nullptr) {
		r->InitWithTargetTypeId(TYPE_ID_UINT8);
	} else if (ctx->INT_U16() != nullptr) {
		r->InitWithTargetTypeId(TYPE_ID_UINT16);
	} else if (ctx->INT_U32() != nullptr) {
		r->InitWithTargetTypeId(TYPE_ID_UINT32);
	} else if (ctx->INT_U64() != nullptr) {
		r->InitWithTargetTypeId(TYPE_ID_UINT64);
	} else {
		panicf("bug");
	}
	return r;
}
std::any Visitor::visitType_float(PinlangParser::Type_floatContext* ctx) {
	AstNodeType* r = new AstNodeType();
	if (ctx->FLOAT_F32() != nullptr) {
		r->InitWithTargetTypeId(TYPE_ID_FLOAT32);
	} else if (ctx->FLOAT_F64() != nullptr) {
		r->InitWithTargetTypeId(TYPE_ID_FLOAT64);
	} else {
		panicf("bug");
	}
	return r;
}
std::any Visitor::visitType_bool(PinlangParser::Type_boolContext* ctx) {
	AstNodeType* r = new AstNodeType();
	r->InitWithTargetTypeId(TYPE_ID_BOOL);
	return r;
}
std::any Visitor::visitType_pointer(PinlangParser::Type_pointerContext* ctx) {
	AstNodeType* pointee_type = std::any_cast<AstNodeType*>(ctx->type()->accept(this));
	AstNodeType* r			  = new AstNodeType();
	r->InitWithPointer(pointee_type);
	return r;
}
std::any Visitor::visitType(PinlangParser::TypeContext* ctx) {
	if (ctx->TYPE() != nullptr) {
		AstNodeType* r = new AstNodeType();
		r->InitWithType();
		return r;
	} else if (ctx->Identifier() != nullptr) {
		AstNodeType* r = new AstNodeType();
		r->InitWithIdentifier(ctx->Identifier()->getText());
		return r;
	} else if (ctx->type_integer() != nullptr) {
		return ctx->type_integer()->accept(this);
	} else if (ctx->type_float() != nullptr) {
		return ctx->type_float()->accept(this);
	} else if (ctx->type_bool() != nullptr) {
		return ctx->type_bool()->accept(this);
	} else if (ctx->type_array() != nullptr) {
		return ctx->type_array()->accept(this);
	} else if (ctx->type_pointer() != nullptr) {
		return ctx->type_pointer()->accept(this);
	} else {
		panicf("unknown type");
	}
}
std::any Visitor::visitType_list(PinlangParser::Type_listContext* ctx) {
	std::vector<AstNodeType*> type_list;
	for (auto iter : ctx->type()) {
		type_list.push_back(std::any_cast<AstNodeType*>(iter->accept(this)));
	}
	return type_list;
}
// TODO 根据返回来确定字面值是32bit/64bit
std::any Visitor::visitExpr_primary_literal(PinlangParser::Expr_primary_literalContext* ctx) {
	auto literal = ctx->literal();
	if (literal->IntegerLiteral() != nullptr) {
		AstNodeType* type = nullptr;
		std::string	 text = literal->IntegerLiteral()->getText();
		std::string	 value_s;
		if (has_suffix(text, "i8")) {
			value_s = text.substr(0, text.length() - 2);
			type	= new AstNodeType();
			type->InitWithTargetTypeId(TYPE_ID_INT8);
		} else if (has_suffix(text, "i16")) {
			value_s = text.substr(0, text.length() - 3);
			type	= new AstNodeType();
			type->InitWithTargetTypeId(TYPE_ID_INT16);
		} else if (has_suffix(text, "i32")) {
			value_s = text.substr(0, text.length() - 3);
			type	= new AstNodeType();
			type->InitWithTargetTypeId(TYPE_ID_INT32);
		} else if (has_suffix(text, "i64")) {
			value_s = text.substr(0, text.length() - 3);
			type	= new AstNodeType();
			type->InitWithTargetTypeId(TYPE_ID_INT64);
		} else if (has_suffix(text, "u8")) {
			value_s = text.substr(0, text.length() - 2);
			type	= new AstNodeType();
			type->InitWithTargetTypeId(TYPE_ID_UINT8);
		} else if (has_suffix(text, "u16")) {
			value_s = text.substr(0, text.length() - 3);
			type	= new AstNodeType();
			type->InitWithTargetTypeId(TYPE_ID_UINT16);
		} else if (has_suffix(text, "u32")) {
			value_s = text.substr(0, text.length() - 3);
			type	= new AstNodeType();
			type->InitWithTargetTypeId(TYPE_ID_UINT32);
		} else if (has_suffix(text, "u64")) {
			value_s = text.substr(0, text.length() - 3);
			type	= new AstNodeType();
			type->InitWithTargetTypeId(TYPE_ID_UINT64);
		} else {
			value_s = text;
		}
		uint64_t value = str_to_uint64(value_s);
		if (literal->SUB() != nullptr) {
			value = -1 * value;
		}
		return (AstNode*)new AstNodeLiteralInteger(type, value);
	} else if (literal->FloatLiteral() != nullptr) {
		AstNodeType* type = nullptr;
		std::string	 text = literal->FloatLiteral()->getText();
		std::string	 value_s;
		if (has_suffix(text, "f32")) {
			value_s = text.substr(0, text.length() - 3);
			type	= new AstNodeType();
			type->InitWithIdentifier("f32");
		} else if (has_suffix(text, "f64")) {
			value_s = text.substr(0, text.length() - 3);
			type	= new AstNodeType();
			type->InitWithIdentifier("f64");
		} else {
			value_s = text;
		}

		float value = str_to_float(value_s);
		if (literal->SUB() != nullptr) {
			value = -1 * value;
		}
		return (AstNode*)new AstNodeLiteralFloat(type, value);
	} else if (literal->BoolLiteral() != nullptr) {
		bool value = str_to_bool(literal->BoolLiteral()->getText());
		return (AstNode*)new AstNodeLiteralBool(value);
	} else if (literal->StringLiteral() != nullptr) {
		std::string value = literal->StringLiteral()->getText();
		assert(value.size() >= 2 && value.at(0) == '"' && value.at(value.size() - 1) == '"');
		value = value.substr(1, value.size() - 2);
		replace_str(value, "\\n", "\n");
		replace_str(value, "\\t", "\t");
		replace_str(value, "\\\"", "\"");
		return (AstNode*)new AstNodeLiteralStr(value);
	} else if (literal->PointerLiteral() != nullptr) {
		AstNodeLiteralNullPtr* n = new AstNodeLiteralNullPtr();
		return (AstNode*)n;
	} else {
		panicf("unknown literal");
	}
	return nullptr;
}
std::any Visitor::visitExpr_primary_identifier(PinlangParser::Expr_primary_identifierContext* ctx) {
	return (AstNode*)new AstNodeIdentifier(ctx->getText());
}
/*
 * return std::vector<AstNode*>
 */
std::any Visitor::visitExpr_list(PinlangParser::Expr_listContext* ctx) {
	std::vector<AstNode*> expr_list;
	for (auto iter : ctx->expr()) {
		expr_list.push_back(std::any_cast<AstNode*>(iter->accept(this)));
	}
	return expr_list;
}
std::any Visitor::visitExpr_primary_parens(PinlangParser::Expr_primary_parensContext* ctx) {
	std::vector<AstNode*> expr_list = std::any_cast<std::vector<AstNode*>>(ctx->expr_list()->accept(this));

	if (expr_list.size() == 1) {
		return expr_list.at(0);
	} else if (expr_list.size() > 1) {
		// tuple
		return (AstNode*)new AstNodeTuple(expr_list);
	} else {
		panicf("bug: empty expr list");
	}
}
std::any Visitor::visitExpr_relational(PinlangParser::Expr_relationalContext* ctx) {
	AstNode*	left  = std::any_cast<AstNode*>(ctx->expr().at(0)->accept(this));
	AstNode*	right = std::any_cast<AstNode*>(ctx->expr().at(1)->accept(this));
	std::string constraint_name;
	std::string op;
	SourceInfo	si;
	if (ctx->EQUALS() != NULL) {
		constraint_name = "Equal";
		op				= "equal";
		si.Init(ctx->EQUALS()->getSymbol());
	} else if (ctx->LESS() != NULL) {
		constraint_name = "LessThan";
		op				= "lessThan";
		si.Init(ctx->LESS()->getSymbol());
	} else if (ctx->LESS_OR_EQUALS() != NULL) {
		constraint_name = "LessEqual";
		op				= "lessEqual";
		si.Init(ctx->LESS_OR_EQUALS()->getSymbol());
	} else if (ctx->GREATER() != NULL) {
		constraint_name = "GreaterThan";
		op				= "greaterThan";
		si.Init(ctx->GREATER()->getSymbol());
	} else if (ctx->GREATER_OR_EQUALS() != NULL) {
		constraint_name = "GreaterEqual";
		op				= "greaterEqual";
		si.Init(ctx->GREATER_OR_EQUALS()->getSymbol());
	} else if (ctx->NOT_EQUALS() != NULL) {
		constraint_name = "NotEqual";
		op				= "notEqual";
		si.Init(ctx->NOT_EQUALS()->getSymbol());
	} else {
		panicf("unknown op");
	}
	AstNodeOperator* node = new AstNodeOperator(left, constraint_name, op, right);
	node->SetSourceInfo_op(si);
	return (AstNode*)node;
}
std::any Visitor::visitExpr_muliplicative(PinlangParser::Expr_muliplicativeContext* ctx) {
	AstNode*	left  = std::any_cast<AstNode*>(ctx->expr().at(0)->accept(this));
	AstNode*	right = std::any_cast<AstNode*>(ctx->expr().at(1)->accept(this));
	std::string constraint_name;
	std::string op;
	SourceInfo	si;
	if (ctx->MUL() != NULL) {
		constraint_name = "Mul";
		op				= "mul";
		si.Init(ctx->MUL()->getSymbol());
	} else if (ctx->DIV() != NULL) {
		constraint_name = "Div";
		op				= "div";
		si.Init(ctx->DIV()->getSymbol());
	} else if (ctx->MOD() != NULL) {
		constraint_name = "Mod";
		op				= "mod";
		si.Init(ctx->MOD()->getSymbol());
	} else {
		panicf("unknown op");
	}
	AstNodeOperator* node = new AstNodeOperator(left, constraint_name, op, right);
	node->SetSourceInfo_op(si);
	return (AstNode*)node;
}
std::any Visitor::visitExpr_logical(PinlangParser::Expr_logicalContext* ctx) {
	AstNode*	left  = std::any_cast<AstNode*>(ctx->expr().at(0)->accept(this));
	AstNode*	right = std::any_cast<AstNode*>(ctx->expr().at(1)->accept(this));
	std::string op;
	if (ctx->LOGICAL_AND() != nullptr) {
		op = "and";
	} else if (ctx->LOGICAL_OR() != nullptr) {
		op = "or";
	} else {
		op = "not";
	}

	return (AstNode*)new AstNodeLogicalOperator(left, op, right);
}
std::any Visitor::visitExpr_additive(PinlangParser::Expr_additiveContext* ctx) {
	AstNode*	left  = std::any_cast<AstNode*>(ctx->expr().at(0)->accept(this));
	AstNode*	right = std::any_cast<AstNode*>(ctx->expr().at(1)->accept(this));
	std::string constraint_name;
	std::string op;
	SourceInfo	si;
	if (ctx->ADD() != nullptr) {
		constraint_name = "Add";
		op				= "add";
		si.Init(ctx->ADD()->getSymbol());
	} else if (ctx->SUB() != nullptr) {
		constraint_name = "Sub";
		op				= "sub";
		si.Init(ctx->SUB()->getSymbol());
	} else {
		panicf("unknown op");
	}
	AstNodeOperator* node = new AstNodeOperator(left, constraint_name, op, right);
	return (AstNode*)node;
}
std::any Visitor::visitExpr_primary_expr(PinlangParser::Expr_primary_exprContext* ctx) {
	return ctx->expr_primary()->accept(this);
}
std::any Visitor::visitExpr_primary_init(PinlangParser::Expr_primary_initContext* ctx) {
	return ctx->expr_init()->accept(this);
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
	} else if (ctx->stmt_assignment() != nullptr) {
		return ctx->stmt_assignment()->accept(this);
	} else if (ctx->stmt_for() != nullptr) {
		return ctx->stmt_for()->accept(this);
	} else if (ctx->stmt_while() != nullptr) {
		return ctx->stmt_while()->accept(this);
	} else if (ctx->stmt_break() != nullptr) {
		return ctx->stmt_break()->accept(this);
	} else if (ctx->stmt_continue() != nullptr) {
		return ctx->stmt_continue()->accept(this);
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
// return FnAttr
std::any Visitor::visitFn_attr(PinlangParser::Fn_attrContext* context) {
	FnAttr attr = FnAttr::FN_ATTR_NONE;
	if (context->CONSTRUCTOR().size() > 0) {
		attr = (FnAttr)(attr | FnAttr::FN_ATTR_CONSTRUCTOR);
	}
	if (context->STATIC().size() > 0) {
		attr = (FnAttr)(attr | FnAttr::FN_ATTR_STATIC);
	}
	return attr;
}
/*
 * 解析一个函数定义
 * @return AstNode* (实际是AstNodeComplexFnDef*)
 */
std::any Visitor::visitStmt_simple_fndef(PinlangParser::Stmt_simple_fndefContext* ctx) {
	std::string					 fn_name	 = ctx->Identifier()->getText();
	FnAttr						 fn_attr	 = std::any_cast<FnAttr>(ctx->fn_attr()->accept(this));
	std::vector<ParserParameter> params		 = std::any_cast<std::vector<ParserParameter>>(ctx->parameter_list()->accept(this));
	AstNodeBlockStmt*			 body		 = dynamic_cast<AstNodeBlockStmt*>(std::any_cast<AstNode*>(ctx->stmt_block()->accept(this)));
	AstNodeType*				 return_type = nullptr;
	if (ctx->type() != nullptr) {
		return_type = std::any_cast<AstNodeType*>(ctx->type()->accept(this));
	}

	AstNodeComplexFnDef::Implement				implement(std::vector<ParserGenericParam>(), params, return_type, body);
	std::vector<AstNodeComplexFnDef::Implement> implements;
	implements.push_back(implement);

	return (AstNode*)new AstNodeComplexFnDef(fn_name, implements, fn_attr);

	// return (AstNode*)new AstNodeFnDef(fn_name, params, return_type, body);
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
std::any Visitor::visitExpr_list_optional(PinlangParser::Expr_list_optionalContext* ctx) {
	std::vector<AstNode*> list;
	for (auto iter : ctx->expr()) {
		list.push_back(std::any_cast<AstNode*>(iter->accept(this)));
	}
	return list;
}
std::any Visitor::visitExpr_primary_fncall(PinlangParser::Expr_primary_fncallContext* ctx) {
	AstNode* fn_expr = std::any_cast<AstNode*>(ctx->expr_primary()->accept(this));

	std::vector<AstNode*> list = std::any_cast<std::vector<AstNode*>>(ctx->expr_list_optional()->accept(this));

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
	FnAttr							fn_attr = std::any_cast<FnAttr>(ctx->fn_attr()->accept(this));
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

	AstNodeComplexFnDef::Implement				implement(generic_params, params, return_type, body);
	std::vector<AstNodeComplexFnDef::Implement> implements;
	implements.push_back(implement);
	return (AstNode*)new AstNodeComplexFnDef(fn_name, implements, fn_attr);
	// return (AstNode*)new AstNodeGenericFnDef(fn_name, generic_params, params, return_type, body);
}
// return std::vector<std::string>
std::any Visitor::visitIdentifier_list(PinlangParser::Identifier_listContext* ctx) {
	std::vector<std::string> ids;
	for (auto iter : ctx->Identifier()) {
		ids.push_back(iter->getText());
	}
	return ids;
}
// std::any Visitor::visitExpr_primary_gparam(PinlangParser::Expr_primary_gparamContext* ctx) {
//	std::string				  id		= ctx->Identifier()->getText();
//	std::vector<AstNodeType*> type_list = std::any_cast<std::vector<AstNodeType*>>(ctx->type_list()->accept(this));
//	return (AstNode*)new AstNodeGenericInstantiate(id, type_list);
// }
std::any Visitor::visitExpr_primary_access_attr(PinlangParser::Expr_primary_access_attrContext* ctx) {
	AstNode*	expr	  = std::any_cast<AstNode*>(ctx->expr_primary()->accept(this));
	std::string attr_name = ctx->Identifier()->getText();
	return (AstNode*)new AstNodeAccessAttr(expr, attr_name);
}
std::any Visitor::visitExpr_dereference(PinlangParser::Expr_dereferenceContext* ctx) {
	AstNode*			value_expr = std::any_cast<AstNode*>(ctx->expr_primary()->accept(this));
	AstNodeDereference* n		   = new AstNodeDereference(value_expr);
	return (AstNode*)n;
}
std::any Visitor::visitExpr_reference(PinlangParser::Expr_referenceContext* ctx) {
	AstNode*		  value_expr = std::any_cast<AstNode*>(ctx->expr_primary()->accept(this));
	AstNodeReference* n			 = new AstNodeReference(value_expr);
	return (AstNode*)n;
}
std::any Visitor::visitExpr_type_cast(PinlangParser::Expr_type_castContext* ctx) {
	AstNode*	 expr	  = std::any_cast<AstNode*>(ctx->expr_primary()->accept(this));
	AstNodeType* dst_type = std::any_cast<AstNodeType*>(ctx->type()->accept(this));
	return (AstNode*)new AstNodeTypeCast(expr, dst_type);
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

	AstNodeComplexFnDef::Implement implement(generic_params, params, return_type, body);

	return implement;
}
std::any Visitor::visitStmt_complex_fndef(PinlangParser::Stmt_complex_fndefContext* ctx) {
	std::string									fn_name = ctx->Identifier()->getText();
	FnAttr										fn_attr = std::any_cast<FnAttr>(ctx->fn_attr()->accept(this));
	std::vector<AstNodeComplexFnDef::Implement> implements;
	for (auto iter : ctx->stmt_complex_fndef_implement()) {
		AstNodeComplexFnDef::Implement implement = std::any_cast<AstNodeComplexFnDef::Implement>(iter->accept(this));
		implements.push_back(implement);
	}
	return (AstNode*)new AstNodeComplexFnDef(fn_name, implements, fn_attr);
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
				.name = ids.at(i + 1)->getText(),
				.type = std::any_cast<AstNodeType*>(types.at(i)->accept(this)),
			};
			class_field_list.push_back(field);
		}
	}

	std::vector<AstNodeComplexFnDef*> methods;
	for (auto iter : ctx->stmt_fndef()) {
		AstNode*			 astnode = std::any_cast<AstNode*>(iter->accept(this));
		AstNodeComplexFnDef* n		 = dynamic_cast<AstNodeComplexFnDef*>(astnode);
		assert(n != nullptr);
		if (n->GetName() == class_name) {
			n->SetFnAttr(FnAttr(n->GetFnAttr() | FnAttr::FN_ATTR_CONSTRUCTOR));
		}
		methods.push_back(n);
	}

	std::vector<AstNode*> class_subclass_def_list;
	for (auto iter : ctx->stmt_class_def()) {
		class_subclass_def_list.push_back(std::any_cast<AstNode*>(iter->accept(this)));
	}

	std::vector<ParserClassImplConstraint> class_impl_constraint_list;
	for (auto iter : ctx->stmt_class_def_impl_constraint()) {
		class_impl_constraint_list.push_back(std::any_cast<ParserClassImplConstraint>(iter->accept(this)));
	}

	return (AstNode*)new AstNodeClassDef(class_name, class_field_list, methods, class_subclass_def_list, class_impl_constraint_list);
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
std::any Visitor::visitStmt_assignment(PinlangParser::Stmt_assignmentContext* ctx) {
	AstNode* left  = std::any_cast<AstNode*>(ctx->expr().at(0)->accept(this));
	AstNode* right = std::any_cast<AstNode*>(ctx->expr().at(1)->accept(this));

	return (AstNode*)new AstNodeAssignment(left, right);
}
/*
 * return ParserInitElement
 */
std::any Visitor::visitExpr_init_element(PinlangParser::Expr_init_elementContext* ctx) {
	ParserInitElement element;

	if (ctx->expr_init_body() != nullptr) {
		element.attr_value = std::any_cast<AstNodeInit*>(ctx->expr_init_body()->accept(this));
	} else if (ctx->expr() != nullptr) {
		element.attr_value = std::any_cast<AstNode*>(ctx->expr()->accept(this));
		if (ctx->Identifier() != nullptr) {
			element.attr_name = ctx->Identifier()->getText();
		}
	} else {
		panicf("bug");
	}

	return element;
}
/*
 * return AstNodeInit*
 */
std::any Visitor::visitExpr_init_body(PinlangParser::Expr_init_bodyContext* ctx) {
	std::vector<ParserInitElement> elements;
	for (auto iter : ctx->expr_init_element()) {
		elements.push_back(std::any_cast<ParserInitElement>(iter->accept(this)));
	}

	return new AstNodeInit(elements);
}
/*
 * return AstNodeInit*
 */
std::any Visitor::visitExpr_init(PinlangParser::Expr_initContext* ctx) {
	AstNodeInit* astnode_init = std::any_cast<AstNodeInit*>(ctx->expr_init_body()->accept(this));
	AstNodeType* astnode_type = nullptr;

	if (ctx->TYPE() != nullptr) {
		astnode_type = new AstNodeType();
		astnode_type->InitWithType();
	} else if (ctx->Identifier() != nullptr) {
		astnode_type = new AstNodeType();
		astnode_type->InitWithIdentifier(ctx->Identifier()->getText());
	} else if (ctx->type_integer() != nullptr) {
		astnode_type = std::any_cast<AstNodeType*>(ctx->type_integer()->accept(this));
	} else if (ctx->type_float() != nullptr) {
		astnode_type = std::any_cast<AstNodeType*>(ctx->type_float()->accept(this));
	} else if (ctx->type_bool() != nullptr) {
		astnode_type = std::any_cast<AstNodeType*>(ctx->type_bool()->accept(this));
	} else if (ctx->type_array_size_optional() != nullptr) {
		astnode_type = std::any_cast<AstNodeType*>(ctx->type_array_size_optional()->accept(this));
	} else if (ctx->type_pointer() != nullptr) {
		astnode_type = std::any_cast<AstNodeType*>(ctx->type_pointer()->accept(this));
	} else {
		panicf("unknown type");
	}

	astnode_init->SetType(astnode_type);
	return (AstNode*)astnode_init;
}
/*
 * return ParserClassImplConstraint
 */
std::any Visitor::visitStmt_class_def_impl_constraint(PinlangParser::Stmt_class_def_impl_constraintContext* ctx) {
	ParserClassImplConstraint impl;

	impl.constraint_name = ctx->Identifier()->getText();
	if (ctx->identifier_list() != nullptr) {
		std::vector<std::string> gparam_ids = std::any_cast<std::vector<std::string>>(ctx->identifier_list()->accept(this));
		for (auto id : gparam_ids) {
			AstNodeType* type = new AstNodeType();
			type->InitWithIdentifier(id);
			impl.constraint_gparams.push_back(type);
		}
	}

	for (auto iter : ctx->stmt_simple_fndef()) {
		AstNodeComplexFnDef* fn = dynamic_cast<AstNodeComplexFnDef*>(std::any_cast<AstNode*>(iter->accept(this)));
		impl.constraint_fns.push_back(fn);
	}

	return impl;
}
std::any Visitor::visitStmt_for_init(PinlangParser::Stmt_for_initContext* ctx) {
	if (ctx->expr() != nullptr) {
		return ctx->expr()->accept(this);
	} else if (ctx->stmt_vardef() != nullptr) {
		return ctx->stmt_vardef()->accept(this);
	} else if (ctx->stmt_assignment() != nullptr) {
		return ctx->stmt_assignment()->accept(this);
	} else {
		return (AstNode*)nullptr;
	}
}
std::any Visitor::visitStmt_for_cond(PinlangParser::Stmt_for_condContext* ctx) {
	if (ctx->expr() != nullptr) {
		return ctx->expr()->accept(this);
	} else {
		return (AstNode*)nullptr;
	}
}
std::any Visitor::visitStmt_for_loop(PinlangParser::Stmt_for_loopContext* ctx) {
	if (ctx->expr() != nullptr) {
		return ctx->expr()->accept(this);
	} else if (ctx->stmt_assignment() != nullptr) {
		return ctx->stmt_assignment()->accept(this);
	} else {
		return (AstNode*)nullptr;
	}
}
std::any Visitor::visitStmt_for(PinlangParser::Stmt_forContext* ctx) {
	AstNode* init_expr = nullptr;
	AstNode* cond_expr = nullptr;
	AstNode* loop_expr = nullptr;
	if (ctx->stmt_for_init() != nullptr) {
		init_expr = std::any_cast<AstNode*>(ctx->stmt_for_init()->accept(this));
	}
	if (ctx->stmt_for_cond() != nullptr) {
		cond_expr = std::any_cast<AstNode*>(ctx->stmt_for_cond()->accept(this));
	}
	if (ctx->stmt_for_loop() != nullptr) {
		loop_expr = std::any_cast<AstNode*>(ctx->stmt_for_loop()->accept(this));
	}
	AstNode* body = std::any_cast<AstNode*>(ctx->stmt_block()->accept(this));
	return (AstNode*)new AstNodeFor(init_expr, cond_expr, loop_expr, body);
}
std::any Visitor::visitStmt_while(PinlangParser::Stmt_whileContext* ctx) {
	AstNode* cond_expr = nullptr;
	if (ctx->stmt_for_cond() != nullptr) {
		cond_expr = std::any_cast<AstNode*>(ctx->stmt_for_cond()->accept(this));
	}
	AstNode* body = std::any_cast<AstNode*>(ctx->stmt_block()->accept(this));
	return (AstNode*)new AstNodeFor(nullptr, cond_expr, nullptr, body);
}
std::any Visitor::visitStmt_break(PinlangParser::Stmt_breakContext* ctx) {
	return (AstNode*)new AstNodeBreak();
}
std::any Visitor::visitStmt_continue(PinlangParser::Stmt_continueContext* context) {
	return (AstNode*)new AstNodeContinue();
}
std::any Visitor::visitExpr_primary_new(PinlangParser::Expr_primary_newContext* ctx) {
	return ctx->expr_new()->accept(this);
}
std::any Visitor::visitExpr_new(PinlangParser::Expr_newContext* ctx) {
	std::vector<AstNode*> args = std::any_cast<std::vector<AstNode*>>(ctx->expr_list_optional()->accept(this));

	if (ctx->Identifier().size() > 0) {
		std::vector<std::string> ids;
		for (auto id : ctx->Identifier()) {
			ids.push_back(id->getText());
		}
		return (AstNode*)new AstNodeNew(ids, args);
	} else if (ctx->type() != nullptr) {
		AstNodeType* ty = std::any_cast<AstNodeType*>(ctx->type()->accept(this));
		return (AstNode*)new AstNodeNew(ty, args);
	} else {
		panicf("bug");
	}
}
