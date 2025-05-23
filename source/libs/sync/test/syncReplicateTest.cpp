#include <gtest/gtest.h>
#include "syncTest.h"

void logTest() {
  sTrace("--- sync log test: trace");
  sDebug("--- sync log test: debug");
  sInfo("--- sync log test: info");
  sWarn("--- sync log test: warn");
  sError("--- sync log test: error");
  sFatal("--- sync log test: fatal");
}

uint16_t    gPorts[] = {7010, 7110, 7210, 7310, 7410};
const char* gDir = "./syncReplicateTest";
int32_t     gVgId = 1234;
SyncIndex   gSnapshotLastApplyIndex;

void init() {
  int code = walInit();
  TD_ALWAYS_ASSERT(code == 0);

  code = syncInit();
  TD_ALWAYS_ASSERT(code == 0);
}

void cleanup() { walCleanUp(); }

void CommitCb(struct SSyncFSM* pFsm, const SRpcMsg* pMsg, SFsmCbMeta cbMeta) {
  SyncIndex beginIndex = SYNC_INDEX_INVALID;
  if (pFsm->FpGetSnapshotInfo != NULL) {
    SSnapshot snapshot;
    pFsm->FpGetSnapshotInfo(pFsm, &snapshot);
    beginIndex = snapshot.lastApplyIndex;
  }

  if (cbMeta.index > beginIndex) {
    char logBuf[256];
    snprintf(logBuf, sizeof(logBuf),
             "==callback== ==CommitCb== pFsm:%p, index:%" PRId64 ", isWeak:%d, code:%d, state:%d %s \n", pFsm,
             cbMeta.index, cbMeta.isWeak, cbMeta.code, cbMeta.state, syncStr(cbMeta.state));
    syncRpcMsgLog2(logBuf, (SRpcMsg*)pMsg);
  } else {
    sTrace("==callback== ==CommitCb== do not apply again %" PRId64, cbMeta.index);
  }
}

void PreCommitCb(struct SSyncFSM* pFsm, const SRpcMsg* pMsg, SFsmCbMeta cbMeta) {
  char logBuf[256];
  snprintf(logBuf, sizeof(logBuf),
           "==callback== ==PreCommitCb== pFsm:%p, index:%" PRId64 ", isWeak:%d, code:%d, state:%d %s \n", pFsm,
           cbMeta.index, cbMeta.isWeak, cbMeta.code, cbMeta.state, syncStr(cbMeta.state));
  syncRpcMsgLog2(logBuf, (SRpcMsg*)pMsg);
}

void RollBackCb(struct SSyncFSM* pFsm, const SRpcMsg* pMsg, SFsmCbMeta cbMeta) {
  char logBuf[256];
  snprintf(logBuf, sizeof(logBuf),
           "==callback== ==RollBackCb== pFsm:%p, index:%" PRId64 ", isWeak:%d, code:%d, state:%d %s \n", pFsm,
           cbMeta.index, cbMeta.isWeak, cbMeta.code, cbMeta.state, syncStr(cbMeta.state));
  syncRpcMsgLog2(logBuf, (SRpcMsg*)pMsg);
}

int32_t GetSnapshotCb(struct SSyncFSM* pFsm, SSnapshot* pSnapshot) {
  pSnapshot->data = NULL;
  pSnapshot->lastApplyIndex = gSnapshotLastApplyIndex;
  pSnapshot->lastApplyTerm = 100;
  return 0;
}

SSyncFSM* createFsm() {
  SSyncFSM* pFsm = (SSyncFSM*)taosMemoryMalloc(sizeof(SSyncFSM));

#if 0 
  pFsm->FpCommitCb = CommitCb;
  pFsm->FpPreCommitCb = PreCommitCb;
  pFsm->FpRollBackCb = RollBackCb;
  pFsm->FpGetSnapshotInfo = GetSnapshotCb;
#endif

  return pFsm;
}

SWal* createWal(char* path, int32_t vgId) {
  SWalCfg walCfg;
  memset(&walCfg, 0, sizeof(SWalCfg));
  walCfg.vgId = vgId;
  walCfg.fsyncPeriod = 1000;
  walCfg.retentionPeriod = 1000;
  walCfg.rollPeriod = 1000;
  walCfg.retentionSize = 1000;
  walCfg.segSize = 1000;
  walCfg.level = TAOS_WAL_FSYNC;
  SWal* pWal = walOpen(path, &walCfg);
  TD_ALWAYS_ASSERT(pWal != NULL);
  return pWal;
}

int64_t createSyncNode(int32_t replicaNum, int32_t myIndex, int32_t vgId, SWal* pWal, char* path) {
  SSyncInfo syncInfo;
  syncInfo.vgId = vgId;
  syncInfo.msgcb = &gSyncIO->msgcb;
  syncInfo.syncSendMSg = syncIOSendMsg;
  syncInfo.syncEqMsg = syncIOEqMsg;
  syncInfo.pFsm = createFsm();
  snprintf(syncInfo.path, sizeof(syncInfo.path), "%s_sync_replica%d_index%d", path, replicaNum, myIndex);
  syncInfo.pWal = pWal;

  SSyncCfg* pCfg = &syncInfo.syncCfg;
  pCfg->myIndex = myIndex;
  pCfg->replicaNum = replicaNum;

  for (int i = 0; i < replicaNum; ++i) {
    pCfg->nodeInfo[i].nodePort = gPorts[i];
    taosGetFqdn(pCfg->nodeInfo[i].nodeFqdn);
    // snprintf(pCfg->nodeInfo[i].nodeFqdn, sizeof(pCfg->nodeInfo[i].nodeFqdn), "%s", "127.0.0.1");
  }

  int64_t rid = syncOpen(&syncInfo);
  TD_ALWAYS_ASSERT(rid > 0);

  SSyncNode* pSyncNode = (SSyncNode*)syncNodeAcquire(rid);
  TD_ALWAYS_ASSERT(pSyncNode != NULL);
  // gSyncIO->FpOnSyncPing = pSyncNode->FpOnPing;
  // gSyncIO->FpOnSyncPingReply = pSyncNode->FpOnPingReply;
  // gSyncIO->FpOnSyncRequestVote = pSyncNode->FpOnRequestVote;
  // gSyncIO->FpOnSyncRequestVoteReply = pSyncNode->FpOnRequestVoteReply;
  // gSyncIO->FpOnSyncAppendEntries = pSyncNode->FpOnAppendEntries;
  // gSyncIO->FpOnSyncAppendEntriesReply = pSyncNode->FpOnAppendEntriesReply;
  // gSyncIO->FpOnSyncPing = pSyncNode->FpOnPing;
  // gSyncIO->FpOnSyncPingReply = pSyncNode->FpOnPingReply;
  // gSyncIO->FpOnSyncTimeout = pSyncNode->FpOnTimeout;
  // gSyncIO->FpOnSyncClientRequest = pSyncNode->FpOnClientRequest;
  gSyncIO->pSyncNode = pSyncNode;
  syncNodeRelease(pSyncNode);

  return rid;
}

void usage(char* exe) { printf("usage: %s replicaNum myIndex lastApplyIndex writeRecordNum \n", exe); }

SRpcMsg* createRpcMsg(int i, int count, int myIndex) {
  SRpcMsg* pMsg = (SRpcMsg*)taosMemoryMalloc(sizeof(SRpcMsg));
  memset(pMsg, 0, sizeof(SRpcMsg));
  pMsg->msgType = 9999;
  pMsg->contLen = 256;
  pMsg->pCont = rpcMallocCont(pMsg->contLen);
  snprintf((char*)(pMsg->pCont), pMsg->contLen, "value-myIndex:%u-%d-%d-%" PRId64, myIndex, i, count,
           taosGetTimestampMs());
  return pMsg;
}

int main(int argc, char** argv) {
  tsAsyncLog = 0;
  sDebugFlag = DEBUG_TRACE + DEBUG_SCREEN + DEBUG_FILE;
  if (argc != 5) {
    usage(argv[0]);
    exit(-1);
  }
  int32_t replicaNum = atoi(argv[1]);
  int32_t myIndex = atoi(argv[2]);
  int32_t lastApplyIndex = atoi(argv[3]);
  int32_t writeRecordNum = atoi(argv[4]);
  gSnapshotLastApplyIndex = lastApplyIndex;

  TD_ALWAYS_ASSERT(replicaNum >= 1 && replicaNum <= 5);
  TD_ALWAYS_ASSERT(myIndex >= 0 && myIndex < replicaNum);
  TD_ALWAYS_ASSERT(lastApplyIndex >= -1);
  TD_ALWAYS_ASSERT(writeRecordNum >= 0);

  init();
  int32_t ret = syncIOStart((char*)"127.0.0.1", gPorts[myIndex]);
  TD_ALWAYS_ASSERT(ret == 0);

  char walPath[128];
  snprintf(walPath, sizeof(walPath), "%s_wal_replica%d_index%d", gDir, replicaNum, myIndex);
  SWal* pWal = createWal(walPath, gVgId);

  int64_t rid = createSyncNode(replicaNum, myIndex, gVgId, pWal, (char*)gDir);
  TD_ALWAYS_ASSERT(rid > 0);
  syncStart(rid);

  SSyncNode* pSyncNode = (SSyncNode*)syncNodeAcquire(rid);
  TD_ALWAYS_ASSERT(pSyncNode != NULL);

  //---------------------------
  int32_t alreadySend = 0;
  while (1) {
    char* s = syncNode2SimpleStr(pSyncNode);

    if (alreadySend < writeRecordNum) {
      SRpcMsg* pRpcMsg = createRpcMsg(alreadySend, writeRecordNum, myIndex);
      int32_t  ret = syncPropose(rid, pRpcMsg, false, NULL);
      if (ret == -1 && terrno == TSDB_CODE_SYN_NOT_LEADER) {
        sTrace("%s value%d write not leader", s, alreadySend);
      } else {
        TD_ALWAYS_ASSERT(ret == 0);
        sTrace("%s value%d write ok", s, alreadySend);
      }
      alreadySend++;

      rpcFreeCont(pRpcMsg->pCont);
      taosMemoryFree(pRpcMsg);
    } else {
      sTrace("%s", s);
    }

    taosMsleep(1000);
    taosMemoryFree(s);
    taosMsleep(1000);
  }

  syncNodeRelease(pSyncNode);
  syncStop(rid);
  walClose(pWal);
  syncIOStop();
  cleanup();
  return 0;
}
