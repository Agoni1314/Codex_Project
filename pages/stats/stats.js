Page({
  data: {
    alarmCount: 0,
    offlineSeconds: 0,
    offlineTimeText: '0 分钟',
    trackCount: 0,
    lastTrackTime: '暂无轨迹',
    riskLevel: '正常',
    riskClass: 'safe'
  },

  onShow() {
    this.loadStats();
    this.stopRefreshTimer();
    this.refreshTimer = setInterval(() => {
      this.loadStats();
    }, 1000);
  },

  onHide() {
    this.stopRefreshTimer();
  },

  onUnload() {
    this.stopRefreshTimer();
  },

  stopRefreshTimer() {
    if (!this.refreshTimer) return;
    clearInterval(this.refreshTimer);
    this.refreshTimer = null;
  },

  loadStats() {
    const app = getApp();
    const globalData = app.globalData || {};
    const globalTracks = Array.isArray(globalData.petTracks) ? globalData.petTracks : [];
    const storageTracks = wx.getStorageSync('petTracks') || [];
    const tracks = storageTracks.length > globalTracks.length ? storageTracks : globalTracks;
    const alarmCount = Number(globalData.alarmCount || wx.getStorageSync('alarmCount') || 0);
    const offlineSeconds = Number(globalData.offlineSeconds || wx.getStorageSync('offlineSeconds') || 0);
    const lastTrack = tracks.length > 0 ? tracks[tracks.length - 1] : null;

    this.setData({
      alarmCount,
      offlineSeconds,
      offlineTimeText: this.formatOfflineTime(offlineSeconds),
      trackCount: tracks.length,
      lastTrackTime: lastTrack ? this.formatTrackTime(lastTrack.time || lastTrack.timestamp) : '暂无轨迹',
      riskLevel: this.getRiskLevel(alarmCount),
      riskClass: this.getRiskClass(alarmCount)
    });
  },

  getRiskLevel(alarmCount) {
    if (alarmCount >= 10) return '高频越界';
    if (alarmCount >= 3) return '需要关注';
    return '正常';
  },

  getRiskClass(alarmCount) {
    if (alarmCount >= 10) return 'danger';
    if (alarmCount >= 3) return 'warning';
    return 'safe';
  },

  formatOfflineTime(seconds) {
    if (!seconds || seconds <= 0) return '0 分钟';

    const hours = Math.floor(seconds / 3600);
    const minutes = Math.floor((seconds % 3600) / 60);

    if (hours > 0) {
      return `${hours} 小时 ${minutes} 分钟`;
    }

    return `${Math.max(minutes, 1)} 分钟`;
  },

  formatTrackTime(time) {
    if (!time) return '未知时间';

    const date = new Date(Number(time));
    if (Number.isNaN(date.getTime())) return '未知时间';

    const pad = (num) => String(num).padStart(2, '0');
    return `${date.getFullYear()}-${pad(date.getMonth() + 1)}-${pad(date.getDate())} ${pad(date.getHours())}:${pad(date.getMinutes())}`;
  },

  clearStats() {
    wx.showModal({
      title: '清空统计',
      content: '仅清空小程序本地统计数据，不会删除硬件中的离线数据。',
      success: (res) => {
        if (!res.confirm) return;

        const app = getApp();
        app.globalData.petTracks = [];
        app.globalData.alarmCount = 0;
        app.globalData.offlineSeconds = 0;

        wx.removeStorageSync('petTracks');
        wx.removeStorageSync('alarmCount');
        wx.removeStorageSync('offlineSeconds');

        this.loadStats();
        wx.showToast({
          title: '已清空',
          icon: 'success'
        });
      }
    });
  }
});
