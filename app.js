App({
  globalData: {
    ble: {
      connected: false,
      deviceId: '',
      deviceName: '',
      serviceId: '',
      writeCharacteristicId: '',
      notifyCharacteristicId: ''
    },
    network: {
      connected: false,
      host: '',
      port: 0
    },
    petTracks: [],
    alarmCount: 0,
    offlineSeconds: 0
  },

  onLaunch() {
    const savedTracks = wx.getStorageSync('petTracks') || [];
    const petTracks = this.removeKnownMockTracks(savedTracks);
    if (petTracks.length !== savedTracks.length) {
      wx.setStorageSync('petTracks', petTracks);
    }

    this.globalData.petTracks = petTracks;
    this.globalData.alarmCount = Number(wx.getStorageSync('alarmCount') || 0);
    this.globalData.offlineSeconds = Number(wx.getStorageSync('offlineSeconds') || 0);
  },

  removeKnownMockTracks(tracks) {
    /*
     * 早期 ESP32 测试程序使用了上海附近的固定模拟坐标：
     * 31.230410, 121.473710，并用 millis() 作为 time。
     *
     * 体验版会保留 wx 本地缓存，用户即使没有连接 GPS，也可能继续看到这批测试点。
     * 这里逐点清理该测试坐标，避免真实轨迹和旧模拟轨迹混合后仍显示上海。
     */
    if (!Array.isArray(tracks) || tracks.length === 0) return [];

    return tracks.filter((item) => !this.isKnownMockTrack(item));
  },

  isKnownMockTrack(item) {
    if (!item) return false;

    const lat = Number(typeof item.lat !== 'undefined' ? item.lat : item.latitude);
    const lng = Number(typeof item.lng !== 'undefined' ? item.lng : item.longitude);
    return lat > 31.22 && lat < 31.25 && lng > 121.46 && lng < 121.49;
  }
});
