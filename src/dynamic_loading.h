#pragma once

#include "define.h"
#include <map>
#include <string>
#include <vector>

class AstNodeBlockStmt;
class AstNodeComplexFnDef;

/*
 * 动态加载动态库
 * 提供三个内置函数:
 *		1. dl_open(path str) int
 *		2. dl_getFn[T Fn](handler int, fn_name str) T
 *		3. dl_close(handler int);
 *  每个加载的动态库有唯一id
 */
class DynamicLoading {
public:
	typedef void* DlHandler;
	struct DynLib {
		std::string path;
		DlHandler	handler;
		int			dynlib_instance_id;

		std::map<std::string, void*> fn_list; // 保存从动态库中获取的符号信息
	};

public:
	DynamicLoading();

	static void RegisterFn(AstNodeBlockStmt& block_stmt);

	// 加载动态库
	bool OpenSharedLibrary(std::string path, int& dynlib_instance_id, std::string& err);
	// 从加载的动态库中获取某个函数. 符号
	bool GetFn(int dynlib_instance_id, std::string fn_name, void** dynlib_fn, std::string& err);
	void CloseSharedLibrary(int id);

	bool FnExist(int dynlib_instance_id, void* fn) const;

	bool SupportType(TypeId tid) const;

	BuiltinFnCallback GetBuiltinFnCallback(std::vector<TypeId> params_tid, TypeId return_tid) const;

private:
	static AstNodeComplexFnDef* create_astnode_fn_dl_open();
	static AstNodeComplexFnDef* create_astnode_fn_dl_getFn();

private:
	std::vector<DynLib> m_dynlib_instances;
};
