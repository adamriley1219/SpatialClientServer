#pragma once
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/EventSystem.hpp"

class ProfilerReport;
class ReportNode;
struct profiler_node_t;
	
void TreeViewSortedByTotalTime();
ProfilerReport* TreeViewSortedByTotalTime(profiler_node_t* root);

void FlatViewSortedBySelfTime();
ProfilerReport* FlatViewSortedBySelfTime(profiler_node_t* root);


std::vector<ProfilerReport*> ProfilerReportGetTreeHistory();
std::vector<ProfilerReport*> ProfilerReportGetFlatHistory();
float ProfilerReportGetAvgFPS();

//--------------------------------------------------------------------------

class ProfilerReport
{
public:
	ProfilerReport( uint64_t timestamp );
	~ProfilerReport();

public:
	void append_flat_view(profiler_node_t* root);
	void append_tree_view(profiler_node_t* root);

	ReportNode* get_root() const;
	typedef bool compare_op(ReportNode const* lhs, ReportNode const* rhs);
	void sort(compare_op op);

	std::string ToString();

private:
	void append(profiler_node_t* toAppend, ReportNode* appendTo);


public:
	ReportNode* head = nullptr;

public:
	bool report_state_tree = true;
	uint64_t total_time = 0;
	uint64_t time_stamp = 0;

};

//--------------------------------------------------------------------------

class ReportNode
{
public:
	ReportNode(ProfilerReport* owner) : m_owner(owner) {};
	ReportNode(ProfilerReport* owner, profiler_node_t* node);
	~ReportNode();

public:
	typedef bool compare_op(ReportNode const* lhs, ReportNode const* rhs);
	void sort_children(compare_op op);

public:
	ReportNode* append(profiler_node_t* toAppend);

public:
	ProfilerReport* m_owner = nullptr;
	ReportNode* m_parent = nullptr;
	std::vector<ReportNode*> m_children;

	std::string m_name = "ProfilerReportRoot";
	uint m_call_count = 1;

	uint64_t m_total_hpc = 0;
	uint64_t m_self_hpc = 0;


	uint64_t m_alloc_count = 0;
	uint64_t m_free_count = 0;
	uint64_t m_bytes_allocated = 0;
	uint64_t m_bytes_freed = 0;


};