const DEFAULT_SERVICE_UUID = '6E400001-B5A3-F393-E0A9-E50E24DCCA9E';
const DEFAULT_WRITE_CHARACTERISTIC_UUID = '6E400002-B5A3-F393-E0A9-E50E24DCCA9E';
const DEFAULT_NOTIFY_CHARACTERISTIC_UUID = '6E400003-B5A3-F393-E0A9-E50E24DCCA9E';

Page({
  data: {
    isBluetoothReady: false,
    isScanning: false,
    connected: false,
    deviceId: '',
    deviceName: '',
    serviceId: '',
    writeCharacteristicId: '',
    notifyCharacteristicId: '',
    statusText: '未连接',
    devices: [],
    offlineTrackCount: 0,
    alarmCount: 0,
    logList: [],

    connectionMode: 'ble',
    wifiHost: '192.168.1.88',
    wifiPort: '8899',
    udpListenPort: '8898',
    udpListening: false,
    wifiDevices: [],
    wifiConnected: false
  },

  incomingText: '',
  wifiIncomingText: '',
  bluetoothModalShown: false,
  tcpSocket: null,
  udpSocket: null,

  onLoad() {
    const app = getApp();
    app.globalData.petTracks = app.globalData.petTracks || wx.getStorageSync('petTracks') || [];
    app.globalData.alarmCount = Number(app.globalData.alarmCount || wx.getStorageSync('alarmCount') || 0);
    app.globalData.offlineSeconds = Number(app.globalData.offlineSeconds || wx.getStorageSync('offlineSeconds') || 0);

    this.setData({
      offlineTrackCount: app.globalData.petTracks.length,
      alarmCount: app.globalData.alarmCount
    });

    this.addLog('页面已加载，准备初始化蓝牙');
    this.listenBluetoothEvents();
    this.openBluetooth();
  },

  onUnload() {
    this.stopScan();
    this.stopWifiDiscovery();
    this.disconnectWifiModule();
  },

  listenBluetoothEvents() {
    wx.onBluetoothAdapterStateChange((res) => {
      this.setData({
        isBluetoothReady: res.available,
        isScanning: res.discovering
      });

      if (!res.available) {
        this.addLog('蓝牙适配器不可用，请检查手机蓝牙');
      }
    });

    wx.onBLEConnectionStateChange((res) => {
      if (res.deviceId !== this.data.deviceId) return;

      if (!res.connected) {
        this.updateGlobalBle(false);
        this.setData({
          connected: false,
          statusText: '离线'
        });

        wx.showToast({
          title: '主人不在家，设备离线记录中',
          icon: 'none',
          duration: 2500
        });

        this.addLog('设备已断开，硬件将本地记录轨迹和报警次数');
      }
    });

    wx.onBLECharacteristicValueChange((res) => {
      if (res.deviceId !== this.data.deviceId) return;
      this.parseBlePacket(res.value);
    });
  },

  openBluetooth() {
    wx.openBluetoothAdapter({
      success: () => {
        this.bluetoothModalShown = false;
        this.setData({ isBluetoothReady: true });
        this.addLog('蓝牙初始化成功');
      },
      fail: (err) => {
        this.setData({
          isBluetoothReady: false,
          isScanning: false
        });

        this.addLog(`蓝牙初始化失败：${err.errMsg}`);

        if (this.bluetoothModalShown) return;
        this.bluetoothModalShown = true;

        wx.showModal({
          title: '蓝牙不可用',
          content: '如果你在微信开发者工具模拟器中测试，BLE 通常不可用，请使用“二维码真机调试”。真机上请开启蓝牙、定位，并允许微信使用蓝牙/附近设备权限。',
          showCancel: false
        });
      }
    });
  },

  startScan() {
    if (!this.data.isBluetoothReady) {
      this.openBluetooth();
      return;
    }

    this.setData({
      devices: [],
      isScanning: true
    });

    const discoveryOptions = {
      allowDuplicatesKey: false,
      services: DEFAULT_SERVICE_UUID ? [DEFAULT_SERVICE_UUID] : undefined,
      success: () => {
        this.addLog('开始扫描附近 BLE 设备');
        wx.onBluetoothDeviceFound((res) => {
          const foundDevices = res.devices || [];
          const devices = this.data.devices.slice();

          foundDevices.forEach((item) => {
            const name = item.name || item.localName || '未知设备';
            const exists = devices.some((device) => device.deviceId === item.deviceId);

            if (!exists) {
              devices.push({
                deviceId: item.deviceId,
                name,
                RSSI: item.RSSI || 0
              });
            }
          });

          this.setData({ devices });
        });
      },
      fail: (err) => {
        this.setData({ isScanning: false });
        this.addLog(`扫描失败：${err.errMsg}`);
      }
    };

    wx.startBluetoothDevicesDiscovery(discoveryOptions);
  },

  stopScan() {
    wx.stopBluetoothDevicesDiscovery({
      complete: () => {
        this.setData({ isScanning: false });
      }
    });
  },

  connectDevice(e) {
    const { deviceId, name } = e.currentTarget.dataset;
    this.stopScan();

    this.setData({
      deviceId,
      deviceName: name,
      statusText: '连接中...'
    });

    wx.createBLEConnection({
      deviceId,
      timeout: 10000,
      success: () => {
        this.setData({
          connected: true,
          statusText: '在线'
        });

        this.addLog(`已连接设备：${name}`);
        this.discoverServiceAndCharacteristics(deviceId);
      },
      fail: (err) => {
        this.setData({
          connected: false,
          statusText: '连接失败'
        });
        this.addLog(`连接失败：${err.errMsg}`);
      }
    });
  },

  discoverServiceAndCharacteristics(deviceId) {
    wx.getBLEDeviceServices({
      deviceId,
      success: (res) => {
        const services = res.services || [];

        if (DEFAULT_SERVICE_UUID) {
          const targetService = services.find((item) => item.uuid.toUpperCase() === DEFAULT_SERVICE_UUID.toUpperCase());
          if (targetService) {
            this.findCharacteristics(deviceId, targetService.uuid);
            return;
          }
        }

        this.findAvailableService(deviceId, services, 0);
      },
      fail: (err) => {
        this.addLog(`获取服务失败：${err.errMsg}`);
      }
    });
  },

  findAvailableService(deviceId, services, index) {
    if (index >= services.length) {
      this.addLog('未找到可写入和可订阅的 BLE 服务');
      return;
    }

    const service = services[index];

    wx.getBLEDeviceCharacteristics({
      deviceId,
      serviceId: service.uuid,
      success: (res) => {
        const characteristics = res.characteristics || [];
        const writeChar = characteristics.find((item) => {
          if (DEFAULT_WRITE_CHARACTERISTIC_UUID) {
            return item.uuid.toUpperCase() === DEFAULT_WRITE_CHARACTERISTIC_UUID.toUpperCase();
          }
          return item.properties.write || item.properties.writeNoResponse;
        });
        const notifyChar = characteristics.find((item) => {
          if (DEFAULT_NOTIFY_CHARACTERISTIC_UUID) {
            return item.uuid.toUpperCase() === DEFAULT_NOTIFY_CHARACTERISTIC_UUID.toUpperCase();
          }
          return item.properties.notify || item.properties.indicate;
        });

        if (writeChar && notifyChar) {
          this.setData({
            serviceId: service.uuid,
            writeCharacteristicId: writeChar.uuid,
            notifyCharacteristicId: notifyChar.uuid
          });
          this.addLog('已找到可通讯的 BLE 特征值');
          this.enableNotify();
        } else {
          this.findAvailableService(deviceId, services, index + 1);
        }
      },
      fail: () => {
        this.findAvailableService(deviceId, services, index + 1);
      }
    });
  },

  findCharacteristics(deviceId, serviceId) {
    wx.getBLEDeviceCharacteristics({
      deviceId,
      serviceId,
      success: (res) => {
        const characteristics = res.characteristics || [];
        const writeChar = characteristics.find((item) => item.uuid.toUpperCase() === DEFAULT_WRITE_CHARACTERISTIC_UUID.toUpperCase());
        const notifyChar = characteristics.find((item) => item.uuid.toUpperCase() === DEFAULT_NOTIFY_CHARACTERISTIC_UUID.toUpperCase());

        if (!writeChar || !notifyChar) {
          this.addLog('固定 UUID 未匹配到，请检查硬件协议配置');
          return;
        }

        this.setData({
          serviceId,
          writeCharacteristicId: writeChar.uuid,
          notifyCharacteristicId: notifyChar.uuid
        });
        this.enableNotify();
      },
      fail: (err) => {
        this.addLog(`获取特征值失败：${err.errMsg}`);
      }
    });
  },

  enableNotify() {
    wx.notifyBLECharacteristicValueChange({
      deviceId: this.data.deviceId,
      serviceId: this.data.serviceId,
      characteristicId: this.data.notifyCharacteristicId,
      state: true,
      success: () => {
        this.updateGlobalBle(true);
        this.addLog('已开启硬件数据通知');
        this.syncOfflineData();
      },
      fail: (err) => {
        this.addLog(`开启通知失败：${err.errMsg}`);
      }
    });
  },

  syncOfflineData() {
    this.writeJson({ cmd: 'SYNC_OFFLINE' });
    this.addLog('已请求同步离线轨迹和报警次数');
  },

  writeJson(payload) {
    if (!this.data.connected) {
      this.addLog('设备未连接，无法发送数据');
      return;
    }

    if (!this.data.serviceId || !this.data.writeCharacteristicId) {
      this.addLog('BLE 写入特征值未准备好');
      return;
    }

    const text = `${JSON.stringify(payload)}\n`;
    const buffer = this.stringToArrayBuffer(text);

    wx.writeBLECharacteristicValue({
      deviceId: this.data.deviceId,
      serviceId: this.data.serviceId,
      characteristicId: this.data.writeCharacteristicId,
      value: buffer,
      success: () => {
        this.addLog(`已发送：${payload.cmd || 'JSON'}`);
      },
      fail: (err) => {
        this.addLog(`发送失败：${err.errMsg}`);
      }
    });
  },

  parseBlePacket(buffer) {
    /*
     * 微信 BLE 通知回调返回的是 ArrayBuffer，不是字符串。
     *
     * 本项目约定硬件发送 UTF-8 JSON 文本，并用换行符 \n 作为一条消息的结束符。
     * BLE 单次通知长度通常较小，硬件可能会把一条 JSON 拆成多包发送。
     *
     * 解析步骤：
     * 1. 先把 ArrayBuffer 转成 UTF-8 字符串。
     * 2. 拼接到 incomingText 缓冲区，避免半包 JSON 直接 JSON.parse 报错。
     * 3. 按 \n 拆分，最后一段如果没有 \n，说明它可能还是半包，继续留在缓冲区。
     * 4. 对完整消息逐条 JSON.parse。
     */
    const chunkText = this.arrayBufferToUtf8String(buffer);
    this.incomingText += chunkText;

    const parts = this.incomingText.split('\n');
    this.incomingText = parts.pop() || '';

    parts.forEach((text) => {
      const messageText = text.trim();
      if (!messageText) return;

      try {
        const message = JSON.parse(messageText);
        this.handleDeviceMessage(message);
      } catch (err) {
        this.addLog('收到无法解析的硬件数据');
        console.warn('BLE JSON parse error:', err, messageText);
      }
    });
  },

  handleDeviceMessage(message) {
    if (message.type === 'offline_sync') {
      const newTracks = this.saveOfflineSync(message);
      this.addLog(`离线数据同步完成：${newTracks.length} 个轨迹点，报警 ${this.data.alarmCount} 次`);
      wx.showToast({
        title: '离线数据已同步',
        icon: 'success'
      });
      return;
    }

    if (message.type === 'track') {
      this.saveRealtimeTrack(message);
      this.addLog('收到实时轨迹点');
      return;
    }

    if (message.type === 'alarm') {
      this.saveAlarmCount(Number(message.alarmCount || this.data.alarmCount + 1));
      this.addLog('收到 BLE 越界报警');
      return;
    }

    if (message.type === 'pong') {
      this.addLog('设备心跳正常');
      return;
    }

    this.addLog(`收到设备消息：${message.type || '未知类型'}`);
  },

  arrayBufferToUtf8String(buffer) {
    /*
     * ArrayBuffer -> UTF-8 字符串。
     *
     * BLE 收到的是二进制字节流。JSON 文本在硬件侧通常会按 UTF-8 编码，
     * 所以这里先把 ArrayBuffer 转成 Uint8Array，再逐字节还原。
     */
    const bytes = new Uint8Array(buffer);

    if (typeof TextDecoder !== 'undefined') {
      return new TextDecoder('utf-8').decode(bytes);
    }

    let binary = '';
    bytes.forEach((byte) => {
      binary += String.fromCharCode(byte);
    });

    try {
      return decodeURIComponent(escape(binary));
    } catch (err) {
      return binary;
    }
  },

  stringToArrayBuffer(text) {
    const buffer = new ArrayBuffer(text.length);
    const dataView = new DataView(buffer);

    for (let i = 0; i < text.length; i += 1) {
      dataView.setUint8(i, text.charCodeAt(i));
    }

    return buffer;
  },

  disconnectDevice() {
    if (!this.data.deviceId) return;

    wx.closeBLEConnection({
      deviceId: this.data.deviceId,
      complete: () => {
        this.updateGlobalBle(false);
        this.setData({
          connected: false,
          statusText: '离线'
        });
        this.addLog('已主动断开设备');
      }
    });
  },

  updateGlobalBle(connected) {
    const app = getApp();
    app.globalData.ble = {
      connected,
      deviceId: this.data.deviceId,
      deviceName: this.data.deviceName,
      serviceId: this.data.serviceId,
      writeCharacteristicId: this.data.writeCharacteristicId,
      notifyCharacteristicId: this.data.notifyCharacteristicId
    };
  },

  switchMode(e) {
    const mode = e.currentTarget.dataset.mode;
    this.setData({
      connectionMode: mode
    });
  },

  onWifiHostInput(e) {
    this.setData({
      wifiHost: e.detail.value
    });
  },

  onWifiPortInput(e) {
    this.setData({
      wifiPort: e.detail.value
    });
  },

  onUdpPortInput(e) {
    this.setData({
      udpListenPort: e.detail.value
    });
  },

  startWifiDiscovery() {
    if (this.udpSocket) {
      this.stopWifiDiscovery();
    }

    const port = Number(this.data.udpListenPort || 8898);
    const udp = wx.createUDPSocket();
    this.udpSocket = udp;

    try {
      udp.bind(port);
    } catch (err) {
      this.addLog(`UDP 监听失败：${err.message || '端口不可用'}`);
      this.udpSocket = null;
      return;
    }

    udp.onMessage((res) => {
      this.parseUdpBroadcast(res);
    });

    this.setData({
      udpListening: true,
      wifiDevices: []
    });

    this.addLog(`正在监听 Wi-Fi 模块广播，UDP 端口 ${port}`);
  },

  stopWifiDiscovery() {
    if (this.udpSocket) {
      this.udpSocket.close();
      this.udpSocket = null;
    }

    if (this.data.udpListening) {
      this.addLog('已停止 Wi-Fi 模块搜索');
    }

    this.setData({
      udpListening: false
    });
  },

  parseUdpBroadcast(res) {
    /*
     * UDP 自动发现逻辑。
     *
     * Wi-Fi 模块需要在同一个局域网里定时广播 JSON，例如：
     * {"type":"device_hello","deviceId":"PET_WIFI_001","name":"宠物定位器","ip":"192.168.1.88","port":8899}\n
     *
     * 小程序监听 UDP 端口后，会收到模块广播的 ArrayBuffer。
     * 这里将 ArrayBuffer 转成 UTF-8 字符串，再解析 JSON。
     *
     * 如果模块没有在 JSON 里带 ip 字段，小程序会尝试使用 UDP 包的 remoteInfo.address。
     */
    const text = this.arrayBufferToUtf8String(res.message).trim();
    if (!text) return;

    try {
      const message = JSON.parse(text);
      if (message.type !== 'device_hello') return;

      const remoteInfo = res.remoteInfo || {};
      const device = {
        deviceId: message.deviceId || message.id || `${remoteInfo.address || 'unknown'}:${message.port || 8899}`,
        name: message.name || '宠物 Wi-Fi 模块',
        ip: message.ip || remoteInfo.address || '',
        port: Number(message.port || 8899),
        lastSeen: Date.now()
      };

      if (!device.ip) {
        this.addLog('发现 Wi-Fi 模块，但缺少 IP 地址');
        return;
      }

      const devices = this.data.wifiDevices.slice();
      const index = devices.findIndex((item) => item.deviceId === device.deviceId || item.ip === device.ip);

      if (index >= 0) {
        devices[index] = device;
      } else {
        devices.unshift(device);
        this.addLog(`发现 Wi-Fi 模块：${device.name} ${device.ip}:${device.port}`);
      }

      this.setData({
        wifiDevices: devices.slice(0, 8)
      });
    } catch (err) {
      this.addLog('收到无法解析的 UDP 广播');
      console.warn('UDP broadcast parse error:', err, text);
    }
  },

  useWifiDevice(e) {
    const { ip, port, name } = e.currentTarget.dataset;

    this.setData({
      wifiHost: ip,
      wifiPort: String(port)
    });

    this.addLog(`已选择 Wi-Fi 模块：${name || ip}`);
    this.connectWifiModule();
  },

  connectWifiModule() {
    if (!this.data.wifiHost || !this.data.wifiPort) {
      wx.showToast({
        title: '请输入 IP 和端口',
        icon: 'none'
      });
      return;
    }

    if (this.tcpSocket) {
      this.tcpSocket.close();
      this.tcpSocket = null;
    }

    const socket = wx.createTCPSocket();
    this.tcpSocket = socket;
    this.wifiIncomingText = '';
    this.addLog(`正在连接 Wi-Fi 模块 ${this.data.wifiHost}:${this.data.wifiPort}`);

    socket.connect({
      address: this.data.wifiHost,
      port: Number(this.data.wifiPort)
    });

    socket.onConnect(() => {
      this.updateGlobalNetwork(true);
      this.setData({
        wifiConnected: true
      });
      this.addLog('Wi-Fi 模块已连接，开始实时接收数据');
      socket.write(this.stringToArrayBuffer('{"cmd":"START_REALTIME"}\n'));
    });

    socket.onMessage((res) => {
      this.parseWifiPacket(res.message);
    });

    socket.onError((err) => {
      this.updateGlobalNetwork(false);
      this.setData({
        wifiConnected: false
      });
      this.addLog(`Wi-Fi 模块连接错误：${err.errMsg || '未知错误'}`);
    });

    socket.onClose(() => {
      this.updateGlobalNetwork(false);
      this.setData({
        wifiConnected: false
      });
      this.addLog('Wi-Fi 模块连接已关闭');
    });
  },

  disconnectWifiModule() {
    if (this.tcpSocket) {
      this.tcpSocket.close();
      this.tcpSocket = null;
    }

    this.updateGlobalNetwork(false);
    this.setData({
      wifiConnected: false
    });
  },

  parseWifiPacket(buffer) {
    /*
     * Wi-Fi TCP 收到的数据和 BLE 一样，底层也是 ArrayBuffer。
     *
     * TCP 有“粘包/半包”问题：一次 onMessage 可能收到半条 JSON，
     * 也可能一次收到多条 JSON。所以这里继续采用：
     * UTF-8 JSON 字符串 + \n 结尾
     *
     * 模块可以实时推送：
     * {"type":"track","lat":31.2304,"lng":121.4737,"time":1783695600000,"alarm":false}\n
     *
     * 小程序收到后会立即写入全局数据和本地缓存，地图页刷新后就能画出新轨迹。
     */
    const chunkText = this.arrayBufferToUtf8String(buffer);
    this.wifiIncomingText += chunkText;

    const parts = this.wifiIncomingText.split('\n');
    this.wifiIncomingText = parts.pop() || '';

    parts.forEach((text) => {
      const messageText = text.trim();
      if (!messageText) return;

      try {
        const message = JSON.parse(messageText);

        if (message.type === 'track') {
          this.saveRealtimeTrack(message);
          this.addLog('Wi-Fi 实时轨迹已记录');
          return;
        }

        if (message.type === 'offline_sync') {
          const newTracks = this.saveOfflineSync(message);
          this.addLog(`Wi-Fi 离线同步完成：${newTracks.length} 个轨迹点`);
          return;
        }

        if (message.type === 'alarm') {
          this.saveAlarmCount(Number(message.alarmCount || this.data.alarmCount + 1));
          this.addLog('收到 Wi-Fi 越界报警');
          return;
        }

        this.addLog(`收到 Wi-Fi 消息：${message.type || '未知类型'}`);
      } catch (err) {
        this.addLog('收到无法解析的 Wi-Fi 数据');
        console.warn('Wi-Fi JSON parse error:', err, messageText);
      }
    });
  },

  saveRealtimeTrack(message) {
    const app = getApp();
    const track = {
      lat: Number(message.lat || message.latitude),
      lng: Number(message.lng || message.longitude),
      time: message.time || message.timestamp || Date.now()
    };

    if (Number.isNaN(track.lat) || Number.isNaN(track.lng)) {
      this.addLog('实时轨迹缺少有效经纬度');
      return;
    }

    if (app.isKnownMockTrack(track)) {
      this.addLog('已忽略旧版上海模拟轨迹');
      return;
    }

    app.globalData.petTracks = app.globalData.petTracks || [];
    app.globalData.petTracks.push(track);

    if (typeof message.alarmCount !== 'undefined') {
      app.globalData.alarmCount = Number(message.alarmCount || 0);
    } else if (message.alarm) {
      app.globalData.alarmCount = Number(app.globalData.alarmCount || 0) + 1;
    }

    wx.setStorageSync('petTracks', app.globalData.petTracks);
    wx.setStorageSync('alarmCount', app.globalData.alarmCount || 0);

    this.setData({
      offlineTrackCount: app.globalData.petTracks.length,
      alarmCount: Number(app.globalData.alarmCount || 0)
    });
  },

  saveOfflineSync(message) {
    const app = getApp();
    const oldTracks = app.globalData.petTracks || [];
    const receivedTracks = Array.isArray(message.tracks) ? message.tracks : [];
    const newTracks = app.removeKnownMockTracks(receivedTracks);

    app.globalData.petTracks = oldTracks.concat(newTracks);
    app.globalData.alarmCount = Number(message.alarmCount || app.globalData.alarmCount || 0);
    app.globalData.offlineSeconds = Number(message.offlineSeconds || app.globalData.offlineSeconds || 0);

    wx.setStorageSync('petTracks', app.globalData.petTracks);
    wx.setStorageSync('alarmCount', app.globalData.alarmCount);
    wx.setStorageSync('offlineSeconds', app.globalData.offlineSeconds);

    this.setData({
      offlineTrackCount: app.globalData.petTracks.length,
      alarmCount: app.globalData.alarmCount
    });

    return newTracks;
  },

  saveAlarmCount(alarmCount) {
    const app = getApp();
    app.globalData.alarmCount = alarmCount;
    wx.setStorageSync('alarmCount', alarmCount);
    this.setData({ alarmCount });
  },

  updateGlobalNetwork(connected) {
    const app = getApp();
    app.globalData.network = {
      connected,
      host: this.data.wifiHost,
      port: Number(this.data.wifiPort),
      socket: connected ? this.tcpSocket : null
    };
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
