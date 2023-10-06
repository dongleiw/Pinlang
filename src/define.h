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
	TYPE_GROUP_ID_NONE	   = 0,
	TYPE_GROUP_ID_PRIMARY  = 1,
	TYPE_GROUP_ID_ARRAY	   = 2,
	TYPE_GROUP_ID_FUNCTION = 3,
	TYPE_GROUP_ID_CLASS	   = 4,
	/*
	 * constraint
	 */
	TYPE_GROUP_ID_CONSTRAINT	= 5,
	TYPE_GROUP_ID_VIRTUAL_GTYPE = 6,
	TYPE_GROUP_ID_TUPLE			= 7, // 元组
	TYPE_GROUP_ID_POINTER		= 8, // 指针
};

/*
 * 类型id (tid)
 * 每个类型都有一个唯一id
 */
enum TypeId {
	TYPE_ID_NONE  = 0,
	TYPE_ID_INFER = 1, // 推导类型. 在Verify阶段, 所有这种类型的变量都将变更为实际具体类型
	TYPE_ID_TYPE  = 2, // type类型.

	TYPE_ID_INT8   = 3,
	TYPE_ID_INT16  = 4,
	TYPE_ID_INT32  = 5,
	TYPE_ID_INT64  = 6,
	TYPE_ID_UINT8  = 7,
	TYPE_ID_UINT16 = 8,
	TYPE_ID_UINT32 = 9,
	TYPE_ID_UINT64 = 10,

	TYPE_ID_FLOAT32 = 11,
	TYPE_ID_FLOAT64 = 12,

	TYPE_ID_BOOL			   = 13,
	TYPE_ID_STR				   = 14,
	TYPE_ID_GENERIC_CONSTRAINT = 15, // 泛型约束
	TYPE_ID_COMPLEX_FN		   = 16, // 复杂函数
	TYPE_ID_NULL			   = 17, // null指针
};

#define is_integer_type(tid) (TYPE_ID_INT8 <= (tid) && (tid) <= TYPE_ID_UINT64)
#define is_unsigned_integer_type(tid) (TYPE_ID_UINT8 <= (tid) && (tid) <= TYPE_ID_UINT64)
#define is_signed_integer_type(tid) (TYPE_ID_INT8 <= (tid) && (tid) <= TYPE_ID_INT64)
int get_integer_bits(TypeId tid);

#define is_float_type(tid) (TYPE_ID_FLOAT32 <= (tid) && (tid) <= TYPE_ID_FLOAT64)

class VerifyContext;
class ExecuteContext;
class Variable;
class AstNode;
class AstNodeComplexFnDef;
class Function;

class AstNodeType;
/*
 * parse得到的函数参数名和类型信息
 */
struct ParserParameter {
	std::string	 name; // 参数名. 为空代表未指定
	AstNodeType* type; // 参数类型
	//bool		 is_hidden_this;

	//ParserParameter(std::string name, AstNodeType* type) : name(name), type(type), is_hidden_this(false) {
	//}
	//ParserParameter(std::string name, AstNodeType* type, bool is_hidden_this) : name(name), type(type), is_hidden_this(is_hidden_this) {
	//}
	ParserParameter DeepClone();
};
/*
 * parse得到的函数参数名和类型信息
 */
struct ParserClassField {
	std::string	 name; // 参数名. 为空代表未指定
	AstNodeType* type; // 参数类型

	ParserClassField DeepClone();
};
/*
 * parse得到的函数声明
 */
struct ParserFnDeclare {
	std::string					 fnname;
	std::vector<ParserParameter> param_list;
	AstNodeType*				 return_type;

	ParserFnDeclare DeepClone();
};
/*
 * parse得到的泛型参数信息. 包括类型名字, 约束
 */
struct ParserGenericParam {
	std::string				  type_name;				 // 泛型的名字. 不可为空
	std::string				  constraint_name;			 // 约束的名字. 可能为空
	std::vector<AstNodeType*> constraint_generic_params; // 约束的参数. 可能为空

	ParserGenericParam DeepClone();
};

/*
 * verify得到的方法的位置
 */
struct MethodIndex {
	MethodIndex() {
		constraint_tid = TYPE_ID_NONE;
		method_idx	   = -1;
	}
	MethodIndex(TypeId constraint_tid, size_t method_idx) {
		this->constraint_tid = constraint_tid;
		this->method_idx	 = (int)method_idx;
	}
	bool IsValid() const { return method_idx >= 0; }

	TypeId constraint_tid; // 方法所在的constraint_tid
	int	   method_idx;	   // 方法在constraint中的下标
};

bool is_vec_typeid_equal(const std::vector<TypeId>& a, const std::vector<TypeId>& b);
bool is_vec_typeid_equal(const std::vector<TypeId>& a, const std::vector<TypeId>& b, size_t shift_a, size_t shift_b);

// 泛参的实际类型
struct ConcreteGParam {
	std::string gparam_name;
	TypeId		gparam_tid;
};

// 初始化表达式中的一个元素
struct ParserInitElement {
	std::string attr_name; // 为空则表示没有指明属性名
	AstNode*	attr_value;

	ParserInitElement DeepClone();
};

// class实现的constraint信息
struct ParserClassImplConstraint {
	std::string						  constraint_name;
	std::vector<AstNodeType*>		  constraint_gparams;
	std::vector<AstNodeComplexFnDef*> constraint_fns;
};

enum FnAttr {
	FN_ATTR_NONE		= 0,
	FN_ATTR_CONSTRUCTOR = 1 << 0,
	FN_ATTR_STATIC		= 1 << 1,
};
void validate_fn_attr(FnAttr attr);

enum FnParamAttr {
	FN_PARAM_ATTR_NONE	   = 0,
	FN_PARAM_ATTR_PTR_SELF = 1 << 0, // 该参数是*self
};
void validate_fn_param_attr(FnParamAttr attr);

// 类型的方法
struct Method {
	std::string			 method_name;
	AstNodeComplexFnDef* method_node;
};
struct ConstraintInstance {
	std::string constraint_name;
	std::string constraint_instance_name;
};
