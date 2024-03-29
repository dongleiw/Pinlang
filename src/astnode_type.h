#pragma once

#include <string>

#include "astnode.h"
#include "define.h"
#include "execute_context.h"
#include "type.h"
#include "variable.h"
#include "verify_context.h"

/*
 * 类型. 在parser阶段, 信息不全导致不易生成typeid. 因此延期到verify阶段
 *
 */
class AstNodeType : public AstNode {
public:
	enum TypeKind {
		TYPE_KIND_NONE,
		TYPE_KIND_TYPE,
		TYPE_KIND_IDENTIFIER,
		TYPE_KIND_FN,
		TYPE_KIND_ARRAY,
		TYPE_KIND_TUPLE,
		TYPE_KIND_TARGET_TYPE_ID,
		TYPE_KIND_POINTER,
	};

public:
	AstNodeType();
	void InitWithType();
	void InitWithIdentifier(std::string id);
	void InitWithFn(std::vector<ParserParameter> params, AstNodeType* return_type);
	/*
	 * 数组类型. 提供数组元素类型和数组大小. 
	 * 如果size_expr为null, 则数组类型为dynamic-size array
	 * 如果size_expr不为null, 则必须是一个编译期常量表达式, 此时数组类型为static-size array
	 */
	void InitWithArray(AstNodeType* element_type, AstNode* size_expr);
	void InitWithPointer(AstNodeType* pointee_type);
	// 固定解析为特定类型id
	// 目前只是为了手动构造AstNodeComplexFnDef
	void InitWithTargetTypeId(TypeId tid);
	void InitWithTuple(std::vector<AstNodeType*> tuple_element_types);

	// 生成类型id
	virtual VerifyContextResult Verify(VerifyContext& ctx, VerifyContextParam vparam) override;
	virtual Variable*			Execute(ExecuteContext& ctx) override { return nullptr; };

	/*
	 * 已知结果类型, 推导出该类型包含的所有类型名字对应的类型
	 * 比如: 
	 *		已知[]fn(a T, b F)类型id为19. 推导出T和F的类型id是多少
	 */
	std::map<std::string, TypeId> InferType(TypeId target_tid) const;

	virtual AstNode* DeepClone() override { return DeepCloneT(); }
	AstNodeType*	 DeepCloneT();

	bool	 IsArrayType() const { return m_type_kind == TYPE_KIND_ARRAY; }
	AstNode* GetArraySizeExpr() const { return m_array_size_expr; }

	bool IsSelf() const;
	bool IsPointerSelf() const;

private:
	void merge_infer_result(std::map<std::string, TypeId>& to, std::map<std::string, TypeId> another) const;

private:
	TypeKind m_type_kind;

	// identifier
	std::string m_id;

	// fn
	std::vector<ParserParameter> m_fn_params;
	AstNodeType*				 m_fn_return_type;

	// 数组
	AstNodeType* m_array_element_type;
	AstNode*	 m_array_size_expr;

	// target type id
	TypeId m_target_tid;

	// tuple
	std::vector<AstNodeType*> m_tuple_element_types;

	// pointer
	AstNodeType* m_pointee_type;
};
