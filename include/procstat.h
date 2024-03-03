#ifndef __PROCSTAT_H__
#define __PROCSTAT_H__

FsOpenNode* openProcStat(const std::shared_ptr<FsNode>& node, U32 flags, U32 data);

#endif