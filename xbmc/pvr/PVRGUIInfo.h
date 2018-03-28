#pragma once
/*
 *      Copyright (C) 2012-2013 Team XBMC
 *      http://kodi.tv
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with XBMC; see the file COPYING.  If not, see
 *  <http://www.gnu.org/licenses/>.
 *
 */

#include <atomic>
#include <string>
#include <vector>

#include "addons/kodi-addon-dev-kit/include/kodi/xbmc_pvr_types.h"
#include "threads/CriticalSection.h"
#include "threads/Thread.h"
#include "utils/Observer.h"

#include "pvr/PVRTypes.h"
#include "pvr/addons/PVRClients.h"

class GUIInfo;

namespace PVR
{
  class CPVRGUIInfo : private CThread,
                      private Observer
  {
  public:
    CPVRGUIInfo(void);
    ~CPVRGUIInfo(void) override;

    void Start(void);
    void Stop(void);

    void Notify(const Observable &obs, const ObservableMessage msg) override;

    bool TranslateBoolInfo(DWORD dwInfo) const;
    bool TranslateCharInfo(const CFileItem *item, DWORD dwInfo, std::string &strValue) const;
    int TranslateIntInfo(const CFileItem *item, DWORD dwInfo) const;

    /*!
     * @brief Get a GUIInfoManager video label.
     * @param item The item to get the label for.
     * @param iLabel The id of the requested label.
     * @param strValue Will be filled with the requested label value.
     * @return True if the requested label value was set, false otherwise.
     */
    bool GetVideoLabel(const CFileItem *item, int iLabel, std::string &strValue) const;

    /*!
     * @brief Get a GUIInfoManager multi info label.
     * @param item The item to get the label for.
     * @param info The GUI info (label id + additional data).
     * @param strValue Will be filled with the requested label value.
     * @return True if the requested label value was set, false otherwise.
     */
    bool GetMultiInfoLabel(const CFileItem *item, const GUIInfo &info, std::string &strValue) const;

    /*!
     * @brief Get a GUIInfoManager seek time label for the currently playing epg tag.
     * @param iSeekSize The seconds to be seeked from the current playback position.
     * @param format The time format for the label.
     * @param strValue Will be filled with the requested label value.
     * @return True if the label value was set, false otherwise.
     */
    bool GetSeekTimeLabel(int iSeekSize, TIME_FORMAT format, std::string &strValue) const;

    /*!
     * @brief Get the total duration of the currently playing epg event or if no epg is
     *        available the current lenght in seconds of the playing Live TV stream.
     * @return The total duration in seconds or 0 if no channel is playing.
     */
    int GetDuration(void) const;

    /*!
     * @brief Get the elapsed time since the start of the currently playing epg event or if
     *        no epg is available since the start of the playback of the current Live TV stream.
     * @return The time in seconds or 0 if no channel is playing.
     */
    int GetElapsedTime(void) const;

    /*!
     * @brief PS_try: Get the position the TimeshiftBuffer starts relative to the start of the 
     * running live TV event in milliseconds 
     * @return The relative position in milliseconds or NULL if no EPG data.
     */ 
    int TimeshiftStartOffset(void) const;
    int TimeshiftBeforeEventTime(void) const;
    int TimeshiftAfterEventTime(void) const;
    bool HasTimeshiftBeforeEvent(void) const;
    bool HasTimeshiftAfterEvent(void) const;
    float TimeshiftStartPercent(void) const;
    float TimeshiftEndPercent(void) const;
    float TimeshiftPlayPercent(void) const;
    float PlayPercent(const CFileItem *item) const;

    /*!
     * @brief Clear the playing EPG tag.
     */
    void ResetPlayingTag(void);

    /*!
     * @brief Get the currently playing EPG tag.
     * @return The currently playing EPG tag or NULL if no EPG tag is playing.
     */
    CPVREpgInfoTagPtr GetPlayingTag() const;

  private:
    class TimerInfo
    {
    public:
      TimerInfo();
      virtual ~TimerInfo() = default;

      void ResetProperties();

      void UpdateTimersCache();
      void UpdateTimersToggle();
      void UpdateNextTimer();

      void CharInfoActiveTimerTitle(std::string &strValue) const { strValue = m_strActiveTimerTitle; }
      void CharInfoActiveTimerChannelName(std::string &strValue) const { strValue = m_strActiveTimerChannelName; }
      void CharInfoActiveTimerChannelIcon(std::string &strValue) const { strValue = m_strActiveTimerChannelIcon; }
      void CharInfoActiveTimerDateTime(std::string &strValue) const { strValue = m_strActiveTimerTime; }
      void CharInfoNextTimerTitle(std::string &strValue) const { strValue = m_strNextRecordingTitle; }
      void CharInfoNextTimerChannelName(std::string &strValue) const { strValue = m_strNextRecordingChannelName; }
      void CharInfoNextTimerChannelIcon(std::string &strValue) const { strValue = m_strNextRecordingChannelIcon; }
      void CharInfoNextTimerDateTime(std::string &strValue) const { strValue = m_strNextRecordingTime; }
      void CharInfoNextTimer(std::string &strValue) const { strValue = m_strNextTimerInfo; }

      bool HasTimers() const { return m_iTimerAmount > 0; }
      bool HasRecordingTimers() const { return m_iRecordingTimerAmount > 0; }
      bool HasNonRecordingTimers() const { return m_iTimerAmount - m_iRecordingTimerAmount > 0; }

    private:
      bool TimerInfoToggle();

      virtual int AmountActiveTimers() = 0;
      virtual int AmountActiveRecordings() = 0;
      virtual std::vector<CFileItemPtr> GetActiveRecordings() = 0;
      virtual CFileItemPtr GetNextActiveTimer() = 0;

      unsigned int m_iTimerAmount;
      unsigned int m_iRecordingTimerAmount;

      std::string m_strActiveTimerTitle;
      std::string m_strActiveTimerChannelName;
      std::string m_strActiveTimerChannelIcon;
      std::string m_strActiveTimerTime;
      std::string m_strNextRecordingTitle;
      std::string m_strNextRecordingChannelName;
      std::string m_strNextRecordingChannelIcon;
      std::string m_strNextRecordingTime;
      std::string m_strNextTimerInfo;

      unsigned int m_iTimerInfoToggleStart;
      unsigned int m_iTimerInfoToggleCurrent;

      CCriticalSection m_critSection;
    };

    class AnyTimerInfo : public TimerInfo
    {
    public:
      AnyTimerInfo() = default;

    private:
      int AmountActiveTimers() override;
      int AmountActiveRecordings() override;
      std::vector<CFileItemPtr> GetActiveRecordings() override;
      CFileItemPtr GetNextActiveTimer() override;
    };

    class TVTimerInfo : public TimerInfo
    {
    public:
      TVTimerInfo() = default;

    private:
      int AmountActiveTimers() override;
      int AmountActiveRecordings() override;
      std::vector<CFileItemPtr> GetActiveRecordings() override;
      CFileItemPtr GetNextActiveTimer() override;
    };

    class RadioTimerInfo : public TimerInfo
    {
    public:
      RadioTimerInfo() = default;

    private:
      int AmountActiveTimers() override;
      int AmountActiveRecordings() override;
      std::vector<CFileItemPtr> GetActiveRecordings() override;
      CFileItemPtr GetNextActiveTimer() override;
    };

    void ResetProperties(void);
    void ClearQualityInfo(PVR_SIGNAL_STATUS &qualityInfo);
    void ClearDescrambleInfo(PVR_DESCRAMBLE_INFO &descrambleInfo);

    void Process(void) override;

    void UpdatePlayingTag(void);
    void UpdateTimersCache(void);
    void UpdateBackendCache(void);
    void UpdateQualityData(void);
    void UpdateDescrambleData(void);
    void UpdateMisc(void);
    void UpdateNextTimer(void);
    void UpdateTimeshift(void);

    void UpdateTimersToggle(void);

    void CharInfoEpgEventDuration(const CFileItem *item, TIME_FORMAT format, std::string &strValue) const;
    void CharInfoEpgEventElapsedTime(const CFileItem *item, TIME_FORMAT format, std::string &strValue) const;
    void CharInfoEpgEventRemainingTime(const CFileItem *item, TIME_FORMAT format, std::string &strValue) const;
    void CharInfoEpgEventFinishTime(const CFileItem *item, TIME_FORMAT format, std::string &strValue) const;
    void CharInfoBackendNumber(std::string &strValue) const;
    void CharInfoTotalDiskSpace(std::string &strValue) const;
    void CharInfoSignal(std::string &strValue) const;
    void CharInfoSNR(std::string &strValue) const;
    void CharInfoBER(std::string &strValue) const;
    void CharInfoUNC(std::string &strValue) const;
    void CharInfoFrontendName(std::string &strValue) const;
    void CharInfoFrontendStatus(std::string &strValue) const;
    void CharInfoBackendName(std::string &strValue) const;
    void CharInfoBackendVersion(std::string &strValue) const;
    void CharInfoBackendHost(std::string &strValue) const;
    void CharInfoBackendDiskspace(std::string &strValue) const;
    void CharInfoBackendChannels(std::string &strValue) const;
    void CharInfoBackendTimers(std::string &strValue) const;
    void CharInfoBackendRecordings(std::string &strValue) const;
    void CharInfoBackendDeletedRecordings(std::string &strValue) const;
    void CharInfoPlayingClientName(std::string &strValue) const;
    void CharInfoEncryption(std::string &strValue) const;
    void CharInfoService(std::string &strValue) const;
    void CharInfoMux(std::string &strValue) const;
    void CharInfoProvider(std::string &strValue) const;
    void CharInfoTimeshiftStartTime(TIME_FORMAT format, std::string &strValue) const;
    void CharInfoTimeshiftEndTime(TIME_FORMAT format, std::string &strValue) const;
    void CharInfoTimeshiftPlayTime(TIME_FORMAT format, std::string &strValue) const;
    void CharInfoTimeshiftOffset(TIME_FORMAT format, std::string &strValue) const;

    int GetRemainingTime(const CFileItem *item) const;

    void CharInfoTimeshiftBeforeEventTime(std::string &strValue) const;
    void CharInfoTimeshiftAfterEventTime(std::string &strValue) const;
    void UpdateInfos(void);
    /** @name GUIInfoManager data */
    //@{
    AnyTimerInfo   m_anyTimersInfo; // tv + radio
    TVTimerInfo    m_tvTimersInfo;
    RadioTimerInfo m_radioTimersInfo;

    bool                            m_bHasTVRecordings;
    bool                            m_bHasRadioRecordings;
    unsigned int                    m_iCurrentActiveClient;
    std::string                     m_strPlayingClientName;
    std::string                     m_strBackendName;
    std::string                     m_strBackendVersion;
    std::string                     m_strBackendHost;
    std::string                     m_strBackendTimers;
    std::string                     m_strBackendRecordings;
    std::string                     m_strBackendDeletedRecordings;
    std::string                     m_strBackendChannels;
    long long                       m_iBackendDiskTotal;
    long long                       m_iBackendDiskUsed;
    unsigned int                    m_iDuration;
    bool                            m_bIsPlayingTV;
    bool                            m_bIsPlayingRadio;
    bool                            m_bIsPlayingRecording;
    bool                            m_bIsPlayingEpgTag;
    bool                            m_bIsPlayingEncryptedStream;
    bool                            m_bHasTVChannels;
    bool                            m_bHasRadioChannels;
    bool                            m_bCanRecordPlayingChannel;
    bool                            m_bIsRecordingPlayingChannel;
    std::string                     m_strPlayingTVGroup;
    std::string                     m_strPlayingRadioGroup;

    //@}

    PVR_SIGNAL_STATUS               m_qualityInfo;       /*!< stream quality information */
    PVR_DESCRAMBLE_INFO             m_descrambleInfo;    /*!< stream descramble information */
    CPVREpgInfoTagPtr               m_playingEpgTag;
    std::vector<SBackend>           m_backendProperties;

    bool                            m_bHasTimeshiftData;
    bool                            m_bIsTimeshifting;
    time_t                          m_iLastTimeshiftUpdate;
    time_t                          m_iStartTime;
    time_t                          m_iTimeshiftStartTime;
    time_t                          m_iTimeshiftEndTime;
    time_t                          m_iTimeshiftPlayTime;
    unsigned int                    m_iTimeshiftOffset;

    unsigned int                    m_iTimeshiftStartOffset;
    unsigned int                    m_iTimeshiftBeforeEventTime;
    unsigned int                    m_iTimeshiftAfterEventTime;
    bool                            m_bHasTimeshiftBeforeEvent;
    bool                            m_bHasTimeshiftAfterEvent;
    std::string                     m_strTimeshiftBeforeEventTime;
    std::string                     m_strTimeshiftAfterEventTime;

    CCriticalSection                m_critSection;

    /**
     * The various backend-related fields will only be updated when this
     * flag is set. This is done to limit the amount of unnecessary
     * backend querying when we're not displaying any of the queried
     * information.
     */
    mutable std::atomic<bool>       m_updateBackendCacheRequested;
  };
}
