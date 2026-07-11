Page({
  data: {
    threshold: 50
  },

  onLoad() {
    const pages = getCurrentPages()
    const prev = pages[pages.length - 2]
    if (prev && prev.data.yuzhi !== undefined) {
      this.setData({ threshold: prev.data.yuzhi })
    }
  },

  onSliderChange(e) {
    this.setData({ threshold: e.detail.value })
  },

  setPreset(e) {
    this.setData({ threshold: parseInt(e.currentTarget.dataset.value) })
  },

  saveThreshold() {
    const pages = getCurrentPages()
    const prev = pages[pages.length - 2]
    if (prev) {
      prev.updateThreshold(this.data.threshold)
      wx.showToast({ title: '阈值已设置', icon: 'success', duration: 1500 })
      setTimeout(() => wx.navigateBack(), 1500)
    }
  },

  cancel() {
    wx.navigateBack()
  }
})
