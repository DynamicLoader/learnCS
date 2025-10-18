import { ref } from 'vue'
import { defineStore } from 'pinia'

export const useUserStore = defineStore('user', {
  state: () => ({
    username: '',
    uid: '',
    role: ''
  }),
  actions: {
    setUser(newUsername: string, newUid: string, newRole: string) {
      this.username = newUsername
      this.uid = newUid
      this.role = newRole
    },
    clearUser() {
      this.username = ''
      this.uid = ''
      this.role = ''
    }
  },
  persist: true, // 开启数据缓存 +++++++
  
})