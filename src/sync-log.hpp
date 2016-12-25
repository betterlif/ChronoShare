/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (c) 2013-2016, Regents of the University of California.
 *
 * This file is part of ChronoShare, a decentralized file sharing application over NDN.
 *
 * ChronoShare is free software: you can redistribute it and/or modify it under the terms
 * of the GNU General Public License as published by the Free Software Foundation, either
 * version 3 of the License, or (at your option) any later version.
 *
 * ChronoShare is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
 * PARTICULAR PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received copies of the GNU General Public License along with
 * ChronoShare, e.g., in COPYING.md file.  If not, see <http://www.gnu.org/licenses/>.
 *
 * See AUTHORS.md for complete list of ChronoShare authors and contributors.
 */

#ifndef SYNC_LOG_H
#define SYNC_LOG_H

#include "db-helper.hpp"
#include <boost/thread/shared_mutex.hpp>
#include <ccnx-name.h>
#include <map>
#include <sync-state.pb.hpp>

typedef boost::shared_ptr<SyncStateMsg> SyncStateMsgPtr;

class SyncLog : public DbHelper
{
public:
  SyncLog(const boost::filesystem::path& path, const Ccnx::Name& localName);

  /**
   * @brief Get local username
   */
  inline const Ccnx::Name&
  GetLocalName() const;

  sqlite3_int64
  GetNextLocalSeqNo(); // side effect: local seq_no will be increased

  // done
  void
  UpdateDeviceSeqNo(const Ccnx::Name& name, sqlite3_int64 seqNo);

  void
  UpdateLocalSeqNo(sqlite3_int64 seqNo);

  Ccnx::Name
  LookupLocator(const Ccnx::Name& deviceName);

  Ccnx::Name
  LookupLocalLocator();

  void
  UpdateLocator(const Ccnx::Name& deviceName, const Ccnx::Name& locator);

  void
  UpdateLocalLocator(const Ccnx::Name& locator);

  // done
  /**
   * Create an entry in SyncLog and SyncStateNodes corresponding to the current state of SyncNodes
   */
  HashPtr
  RememberStateInStateLog();

  // done
  sqlite3_int64
  LookupSyncLog(const std::string& stateHash);

  // done
  sqlite3_int64
  LookupSyncLog(const Hash& stateHash);

  // How difference is exposed will be determined later by the actual protocol
  SyncStateMsgPtr
  FindStateDifferences(const std::string& oldHash, const std::string& newHash,
                       bool includeOldSeq = false);

  SyncStateMsgPtr
  FindStateDifferences(const Hash& oldHash, const Hash& newHash, bool includeOldSeq = false);

  //-------- only used in test -----------------
  sqlite3_int64
  SeqNo(const Ccnx::Name& name);

  sqlite3_int64
  LogSize();

protected:
  void
  UpdateDeviceSeqNo(sqlite3_int64 deviceId, sqlite3_int64 seqNo);

protected:
  Ndnx::Name m_localName;

  sqlite3_int64 m_localDeviceId;

  typedef boost::mutex Mutex;
  typedef boost::unique_lock<Mutex> WriteLock;

  Mutex m_stateUpdateMutex;
};

typedef boost::shared_ptr<SyncLog> SyncLogPtr;

const Ccnx::Name&
SyncLog::GetLocalName() const
{
  return m_localName;
}

#endif // SYNC_LOG_H
