#pragma once

#include <string>
#include <vector>

/*
 * 类型的分类(tgid)
 */
enum TypeGroupId {
	/*
	 * 未完成定义的类别. 由于解析的顺序问题, 可能类型的使用先于类型的定义. 这种会先临时标记为unresolved
	 * 在后续解析到类型定义时, 在修正为实际类型
	 */
	TYPE_GROUP_ID_UNRESOLVE		   = 0,
	TYPE_GROUP_ID_PRIMARY		   = 1,
	TYPE_GROUP_ID_ARRAY			   = 2,
	TYPE_GROUP_ID_GENERIC_FUNCTION = 3,
	TYPE_GROUP_ID_FUNCTION		   = 4,
	TYPE_GROUP_ID_CLASS			   = 5,
	/*
	 * 泛型restriction
	 */
	TYPE_GROUP_ID_RESTRICTION  = 6,
	TYPE_GROUP_ID_GENERIC_TYPE = 8,
};

/*
 * 类型id (tid)
 * 每个类型都有一个唯一id
 */
enum TypeId {
	TYPE_ID_NONE				= 0,
	TYPE_ID_INFER				= 1, // 推导类型. 在Verify阶段, 所有这种类型的变量都将变更为实际具体类型
	TYPE_ID_TYPE				= 2, // type类型.
	TYPE_ID_INT					= 3,
	TYPE_ID_FLOAT				= 4,
	TYPE_ID_STR					= 5,
	TYPE_ID_BOOL				= 6,
	TYPE_ID_GENERIC_RESTRICTION = 7, // 泛型约束
	TYPE_ID_GENERIC_FN			= 8, // 泛型函数
};

class ExecuteContext;
class Variable;

// 指向内置函数的指针
typedef Variable* (*BuiltinFnCallback)(ExecuteContext& ctx, Variable* thisobj, std::vector<Variable*> args);

class AstNodeType;
/*
 * parse得到的函数参数名和类型信息
 */
struct ParserParameter {
	std::string	 name; // 参数名. 为空代表未指定
	AstNodeType* type; // 参数类型
};
/*
 * parse得到的函数声明
 */
struct ParserFnDeclare {
	std::string					 fnname;
	std::vector<ParserParameter> param_list;
	AstNodeType*				 return_type;
};
/*
 * parse得到的泛型参数信息. 包括类型名字, 约束
 */
struct ParserGenericParam {
	std::string				  type_name;				  // 泛型的名字. 不可为空
	std::string				  restriction_name;			  // 约束的名字
	std::vector<AstNodeType*> restriction_generic_params; // 约束的参数. 可能为空
};
