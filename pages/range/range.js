Page({
  data: {
    radius: 20,
    minRadius: 1,
    maxRadius: 50,
    connected: false,
    deviceName: '',
    connectionType: '',
    sending: false,
    logList: []
  },

  onShow() {
    this.refreshConnectionState();
  },

  refreshConnectionState() {
    const app = getApp();
    const ble = app.globalData && app.globalData.ble ? app.globalData.ble : {};
    const network = app.globalData && app.globalData.network ? app.globalData.network : {};
    const savedRadius = wx.getStorageSync('petRangeRadius');
    const radius = savedRadius ? Number(savedRadius) : this.data.radius;
    const isWifiConnected = !!(network.connected && network.host && network.port);
    const isBleConnected = !!ble.connected;

    this.setData({
      connected: isWifiConnected || isBleConnected,
      deviceName: isWifiConnected
        ? `WiFi 设备 ${network.host}:${network.port}`
        : (ble.deviceName || ''),
      connectionType: isWifiConnected ? 'wifi' : (isBleConnected ? 'ble' : ''),
      radius: Math.min(this.data.maxRadius, Math.max(this.data.minRadius, radius))
    });
  },

  onRadiusChange(e) {
    this.setData({
      radius: Number(e.detail.value)
    });
  },

  sendRadiusSetting() {
    const app = getApp();
    const ble = app.globalData && app.globalData.ble ? app.globalData.ble : {};
    const network = app.globalData && app.globalData.network ? app.globalData.network : {};
    const payload = {
      cmd: 'SET_RADIUS',
      radius: this.data.radius
    };

    if (network.connected && network.socket) {
      this.sendRadiusByWifi(network.socket, payload);
      return;
    }

    if (ble.connected && ble.deviceId && ble.serviceId && ble.writeCharacteristicId) {
      this.sendRadiusByBle(ble, payload);
      return;
    }

    wx.showToast({
      title: '请先连接设备',
      icon: 'none'
    });
    this.addLog('下发失败：设备未连接');
    this.refreshConnectionState();
  },

  sendRadiusByWifi(socket, payload) {
    this.setData({ sending: true });

    try {
      socket.write(this.stringToArrayBuffer(`${JSON.stringify(payload)}\n`));
      this.handleSendSuccess('WiFi');
    } catch (err) {
      wx.showToast({
        title: '下发失败',
        icon: 'none'
      });
      this.addLog(`WiFi 下发失败：${err.message || err.errMsg || '未知错误'}`);
    } finally {
      this.setData({ sending: false });
    }
  },

  sendRadiusByBle(ble, payload) {
    this.setData({ sending: true });

    wx.writeBLECharacteristicValue({
      deviceId: ble.deviceId,
      serviceId: ble.serviceId,
      characteristicId: ble.writeCharacteristicId,
      value: this.stringToArrayBuffer(`${JSON.stringify(payload)}\n`),
      success: () => {
        this.handleSendSuccess('BLE');
      },
      fail: (err) => {
        wx.showToast({
          title: '下发失败',
          icon: 'none'
        });
        this.addLog(`BLE 下发失败：${err.errMsg}`);
      },
      complete: () => {
        this.setData({ sending: false });
      }
    });
  },

  handleSendSuccess(channel) {
    wx.setStorageSync('petRangeRadius', this.data.radius);
    wx.showToast({
      title: '设置已下发',
      icon: 'success'
    });
    this.addLog(`${channel} 已下发活动半径：${this.data.radius} 米`);
  },

  stringToArrayBuffer(text) {
    /*
     * 字符串 -> ArrayBuffer。
     *
     * 微信 BLE 写入和 TCP socket.write 都接收 ArrayBuffer。
     * 当前下发内容是以换行符结尾的 JSON 文本，例如：
     * {"cmd":"SET_RADIUS","radius":20}\n
     *
     * 这条命令只包含英文、数字和标点，属于 ASCII 范围。
     * ASCII 在 UTF-8 中也是单字节编码，因此硬件端可以直接按 UTF-8 JSON 解析。
     */
    const buffer = new ArrayBuffer(text.length);
    const dataView = new DataView(buffer);

    for (let i = 0; i < text.length; i += 1) {
      dataView.setUint8(i, text.charCodeAt(i));
    }

    return buffer;
  },

  addLog(text) {
    const time = this.formatTime(new Date());
    const logList = [`${time} ${text}`].concat(this.data.logList).slice(0, 8);
    this.setData({ logList });
  },

  formatTime(date) {
    const pad = (num) => String(num).padStart(2, '0');
    return `${pad(date.getHours())}:${pad(date.getMinutes())}:${pad(date.getSeconds())}`;
  }
});
