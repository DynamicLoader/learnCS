
import './assets/tailwind.css'; // Ensure this is the correct path to your Tailwind CSS file
// import './assets/main.css'
import 'primeicons/primeicons.css';
import './assets/main.css';

import { createApp } from 'vue'
import { createPinia } from 'pinia'
import piniaPluginPersistedstate from 'pinia-plugin-persistedstate'

import App from './App.vue'
import PrimeVue from 'primevue/config'
import Aura from '@primevue/themes/aura'
import ConfirmationService from 'primevue/confirmationservice';
import { ToastService } from 'primevue';
import Ripple from 'primevue/ripple';
import router from './router'


const app = createApp(App)


app.use(PrimeVue, {
    theme: {
        preset: Aura,
    }
})
// const globals = {apiPrefix : 'http://localhost:8080/api/v1'};
// app.provide('globals', globals);
app.use(router)
const pinia = createPinia()
pinia.use(piniaPluginPersistedstate) //++++
app.use(pinia)

app.use(ConfirmationService);
app.use(ToastService);
app.directive('ripple', Ripple);
app.mount('#app')
