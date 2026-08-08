Page({
  data: {
    latitude: 0,
    longitude: 0,
    radius: 20,
    markers: [],
    polyline: [],
    circles: [],
    trackCount: 0,
    lastSyncText: '暂无轨迹',
    locationText: '暂无经纬度'
  },

  mapContext: null,

  onReady() {
    this.mapContext = wx.createMapContext('petMap', this);
  },

  onShow() {
    this.loadMapData();
  },

  loadMapData() {
    const app = getApp();
    const globalTracks = app.globalData && app.globalData.petTracks ? app.globalData.petTracks : [];
    const storageTracks = wx.getStorageSync('petTracks') || [];
    const sourceTracks = storageTracks.length >= globalTracks.length ? storageTracks : globalTracks;
    const tracks = app.removeKnownMockTracks(sourceTracks);
    if (tracks.length !== sourceTracks.length) {
      app.globalData.petTracks = tracks;
      wx.setStorageSync('petTracks', tracks);
    }
    const savedRadius = wx.getStorageSync('petRangeRadius');
    const radius = savedRadius ? Number(savedRadius) : 20;

    if (!tracks.length) {
      this.setEmptyMap(radius);
      return;
    }

    const validTracks = tracks
      .map((item) => ({
        latitude: Number(typeof item.lat !== 'undefined' ? item.lat : item.latitude),
        longitude: Number(typeof item.lng !== 'undefined' ? item.lng : item.longitude),
        time: item.time || item.timestamp || ''
      }))
      .filter((item) => {
        return !Number.isNaN(item.latitude)
          && !Number.isNaN(item.longitude)
          && Math.abs(item.latitude) <= 90
          && Math.abs(item.longitude) <= 180;
      });

    if (!validTracks.length) {
      this.setEmptyMap(radius);
      return;
    }

    const firstPoint = validTracks[0];
    const lastPoint = validTracks[validTracks.length - 1];

    this.setData({
      latitude: lastPoint.latitude,
      longitude: lastPoint.longitude,
      radius,
      trackCount: validTracks.length,
      lastSyncText: this.formatTrackTime(lastPoint.time),
      locationText: `${lastPoint.latitude.toFixed(6)}, ${lastPoint.longitude.toFixed(6)}`,
      markers: [
        {
          id: 1,
          latitude: firstPoint.latitude,
          longitude: firstPoint.longitude,
          title: '活动中心',
          callout: {
            content: '活动中心',
            color: '#12352A',
            fontSize: 13,
            borderRadius: 6,
            bgColor: '#FFFFFF',
            padding: 8,
            display: 'ALWAYS'
          }
        },
        {
          id: 2,
          latitude: lastPoint.latitude,
          longitude: lastPoint.longitude,
          title: '当前位置',
          callout: {
            content: '当前位置',
            color: '#FFFFFF',
            fontSize: 13,
            borderRadius: 6,
            bgColor: '#2E8B57',
            padding: 8,
            display: 'ALWAYS'
          }
        }
      ],
      polyline: [
        {
          points: validTracks,
          color: '#2E8B57',
          width: 5,
          dottedLine: false,
          arrowLine: true
        }
      ],
      circles: [
        {
          latitude: firstPoint.latitude,
          longitude: firstPoint.longitude,
          radius,
          color: '#2E8B57AA',
          fillColor: '#2E8B5726',
          strokeWidth: 2
        }
      ]
    });

    wx.nextTick(() => {
      this.includeTrackPoints(validTracks);
    });
  },

  setEmptyMap(radius) {
    wx.getLocation({
      type: 'gcj02',
      success: (res) => {
        this.renderEmptyMap(radius, res.latitude, res.longitude, '等待设备同步轨迹');
      },
      fail: () => {
        this.renderEmptyMap(radius, 0, 0, '等待定位授权或设备轨迹');
      }
    });
  },

  renderEmptyMap(radius, latitude, longitude, calloutText) {
    this.setData({
      latitude,
      longitude,
      radius,
      trackCount: 0,
      lastSyncText: '暂无轨迹',
      locationText: `${latitude.toFixed(6)}, ${longitude.toFixed(6)}`,
      markers: [
        {
          id: 1,
          latitude,
          longitude,
          title: '当前位置',
          callout: {
            content: calloutText,
            color: '#12352A',
            fontSize: 13,
            borderRadius: 6,
            bgColor: '#FFFFFF',
            padding: 8,
            display: 'ALWAYS'
          }
        }
      ],
      polyline: [],
      circles: [
        {
          latitude,
          longitude,
          radius,
          color: '#2E8B57AA',
          fillColor: '#2E8B5726',
          strokeWidth: 2
        }
      ]
    });
  },

  includeTrackPoints(points) {
    if (!this.mapContext || points.length === 0) return;

    this.mapContext.includePoints({
      points,
      padding: [80, 60, 220, 60]
    });
  },

  refreshMap() {
    this.loadMapData();

    wx.showToast({
      title: '地图已刷新',
      icon: 'success'
    });
  },

  clearLocalTracks() {
    wx.showModal({
      title: '清空轨迹',
      content: '仅清空小程序本地展示数据，不会删除硬件中的离线记录。',
      success: (res) => {
        if (!res.confirm) return;

        const app = getApp();
        app.globalData.petTracks = [];

        wx.removeStorageSync('petTracks');
        this.setEmptyMap(this.data.radius);

        wx.showToast({
          title: '已清空',
          icon: 'success'
        });
      }
    });
  },

  formatTrackTime(time) {
    if (!time) return '未知时间';

    const date = new Date(Number(time));
    if (Number.isNaN(date.getTime())) return '未知时间';

    const pad = (num) => String(num).padStart(2, '0');
    return `${date.getFullYear()}-${pad(date.getMonth() + 1)}-${pad(date.getDate())} ${pad(date.getHours())}:${pad(date.getMinutes())}`;
  }
});
