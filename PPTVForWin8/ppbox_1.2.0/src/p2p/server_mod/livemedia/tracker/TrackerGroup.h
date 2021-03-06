#ifndef _TRACKER_GROUP_H_
#define _TRACKER_GROUP_H_



#include "tracker/TrackerClient.h"

class TrackerGroup
    : public boost::noncopyable
    , public boost::enable_shared_from_this<TrackerGroup>
#ifdef DUMP_OBJECT
    , public count_object_allocate<TrackerGroup>
#endif
{
public:
    typedef boost::shared_ptr<TrackerGroup> p;
    static p Create( framework::timer::AsioTimerManager * asio_timer_manager )
    { 
        return p( new TrackerGroup(asio_timer_manager) );
    }

public:
    void Start();

    void Stop();

    void DoList(const RID& rid);

    void DoQueriedPeerCount(const RID& rid);

    boost::uint32_t GetTrackersNum(const RID& rid);
    void SetTrackers(boost::uint32_t group_count, const std::set<protocol::TRACKER_INFO>& trackers);

    const std::vector<protocol::TRACKER_INFO> GetTrackers();

    void DoCommit();

    void PPLeave();

public:
    void OnTimerElapsed(framework::timer::Timer * pointer);

public:

    void OnListResponsePacket( protocol::ListPacket const & packet);

    void OnReportResponsePacket( protocol::ReportPacket const & packet);

    void OnQueryResponsePacket(protocol::QueryPeerCountPacket const & packet);
public:

    void SetGroupCount(uint32_t group_count);

    uint32_t GetGroupCount() const;

private:

    void StartAllClients();

    void StopAllClients();

    void ClearAllClients();

private:
    /**
    * @brief 根据endpoint来索引TrackerClient
    *    [Key] endpoint （tracker 的 ip + port）
    *    [Value] TrackerClient 的智能指针
    */
    std::map<boost::asio::ip::udp::endpoint, TrackerClient::p> trackers_;

    /**`
    * @brief 当前选定的 登录Tracker， 如果这个指针为空，那么未选定 登录Tracker
    *     注：登录Tracker 是指 需要 Commit 和 KeepAlive 的Tracker
    */
    TrackerClient::p current_tracker_;

public:
    /**
    * @brief 用来查找下一个 Tracker使用的,和 trying_tracker配套使用
    */
    typedef std::list<TrackerClient::p> TrackerClientList; // // 必须使用list
    TrackerClientList tracker_list_;

private:
    /**
    * @brief 用来标记正在尝试Commit/KeepAlive的Tracker, 和tracker_list_配套使用
    **/
    TrackerClient::p trying_tracker_;
    TrackerClientList::iterator trying_tracker_iterator_;

    /**
    * @brief 对当前Tracker Commit或者KeepAlive 的连续出错次数
    */ 
    uint32_t error_times_;

    /**
    * @brief 定期提交 Commit/KeepAlive 的定时器
    */
    framework::timer::PeriodicTimer timer_;

    /**`
    * @brief 上次发出包的 Trans_id
    */
    uint32_t last_transcation_id_;

    /**
    * @发出的包是否, false 为没有收到响应,true 为已收到响应
    */
    bool is_responsed_;

    /**
    * @记录上次收到的Tracker返回的上报资源情况
    */
    // uint32_t last_response_rid_count_;
    uint32_t group_count_;

private:
    static const uint32_t DEFAULT_INTERVAL_IN_SECONDS_ = 20;
    static const uint32_t DEFAULT_INTERVAL_IN_MILLISECONDS_ = 1000 * DEFAULT_INTERVAL_IN_SECONDS_;

private:
    TrackerGroup(framework::timer::AsioTimerManager * asio_timer_manager)
        : timer_(*asio_timer_manager, 1000 * DEFAULT_INTERVAL_IN_SECONDS_, boost::bind(&TrackerGroup::OnTimerElapsed, this, &timer_))

    {}
};


#endif
