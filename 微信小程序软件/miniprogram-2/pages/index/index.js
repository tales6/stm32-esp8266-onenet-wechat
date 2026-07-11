const ALARM_KEYS = ["Alarm", "alarm"]
const parseAlarm = (v) => v === true || v === 1 || v === "1" || v === "true"

Page({
  data: {
    hum: 0,
    yuzhi: 50,
    alarm: false,
    online: false
  },

  config: {
    authorization: "YOUR_ONENET_TOKEN",
    product_id: "YOUR_PRODUCT_ID",
    device_name: "YOUR_DEVICE_NAME",
    property_humidity: "Humidity",
    property_threshold: "Threshold",
    property_alarm: "Alarm"
  },

  applyList(list) {
    const { property_humidity, property_threshold } = this.config
    list.forEach(({ identifier, value }) => {
      if (value === undefined) return
      if (identifier === property_humidity) this.setData({ hum: parseFloat(value) })
      else if (identifier === property_threshold) this.setData({ yuzhi: parseFloat(value) })
      else if (ALARM_KEYS.includes(identifier)) this.setData({ alarm: parseAlarm(value) })
    })
  },

  applyMap(map) {
    const { property_humidity, property_threshold } = this.config
    if (map[property_humidity] !== undefined) this.setData({ hum: parseFloat(map[property_humidity]) })
    if (map[property_threshold] !== undefined) this.setData({ yuzhi: parseFloat(map[property_threshold]) })
    for (const k of Object.keys(map)) {
      if (ALARM_KEYS.includes(k) && map[k] !== undefined) {
        this.setData({ alarm: parseAlarm(map[k]) })
        break
      }
    }
  },

  getHumidity() {
    wx.request({
      url: 'https://iot-api.heclouds.com/thingmodel/query-device-property',
      method: 'GET',
      header: { 'Authorization': this.config.authorization },
      data: {
        product_id: this.config.product_id,
        device_name: this.config.device_name
      },
      success: (res) => {
        if (res.statusCode !== 200 || !res.data || res.data.code !== 0) return
        const d = res.data.data
        if (!d) return
        if (Array.isArray(d)) this.applyList(d)
        else if (Array.isArray(d.properties)) this.applyList(d.properties)
        else if (typeof d === 'object') this.applyMap(d)
      },
      fail: (err) => {
        console.error('获取湿度失败:', err)
      }
    })
  },

  getDeviceStatus() {
    const { product_id, device_name, authorization } = this.config
    wx.request({
      url: `https://iot-api.heclouds.com/device/detail?product_id=${product_id}&device_name=${device_name}`,
      method: 'GET',
      header: { 'Authorization': authorization },
      success: (res) => {
        if (res.data && res.data.code === 0 && res.data.data) {
          this.setData({ online: res.data.data.status === 1 })
        }
      },
      fail: (err) => {
        console.error('获取设备状态失败:', err)
      }
    })
  },

  sendThresholdToDevice(threshold) {
    const url = 'https://iot-api.heclouds.com/thingmodel/set-device-property'
    wx.request({
      url,
      method: 'POST',
      header: {
        'Authorization': this.config.authorization,
        'Content-Type': 'application/json'
      },
      data: {
        product_id: this.config.product_id,
        device_name: this.config.device_name,
        params: { [this.config.property_threshold]: parseInt(threshold) }
      },
      timeout: 10000,
      success: (res) => {
        if (res.data && res.data.code === 0) {
          wx.showToast({ title: '已更新', icon: 'success', duration: 1000 })
        } else {
          wx.showToast({ title: '下发失败: ' + (res.data?.msg || ''), icon: 'none' })
        }
      },
      fail: (err) => {
        console.error('下发失败:', err)
        wx.showToast({ title: '网络错误', icon: 'none' })
      }
    })
  },

  switchPlant(threshold, plantName) {
    this.setData({ yuzhi: threshold })
    wx.setStorageSync('threshold', threshold)
    this.sendThresholdToDevice(threshold)
    wx.showToast({
      title: `已切换${plantName}模式\n湿度阈值${threshold}%`,
      icon: 'success',
      duration: 1500
    })
  },

  updateThreshold(val) {
    this.switchPlant(val, '自定义')
  },

  栀子花() { this.switchPlant(60, '栀子花') },
  绿萝() { this.switchPlant(35, '绿萝') },
  吊兰() { this.switchPlant(50, '吊兰') },
  自定义() { wx.navigateTo({ url: '/pages/custom/custom' }) },

  onLoad() {
    try {
      const saved = wx.getStorageSync('threshold')
      if (saved) this.setData({ yuzhi: saved })
    } catch (_) { /* ignore */ }

    this.getHumidity()
    this.getDeviceStatus()

    setInterval(() => {
      this.getHumidity()
      this.getDeviceStatus()
    }, 3000)
  }
})
