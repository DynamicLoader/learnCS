<script setup lang="ts">

import { useRoute, useRouter } from 'vue-router';
import { useToast } from 'primevue';

const route = useRoute();
const router = useRouter();
const toast = useToast();

if (route.meta.requiresAuth === false) {
  // determine if already logged in
  fetch('/api/auth/login').then(response => response.json()).then(data => {
    if (data.code === 0) {
      toast.add({ severity: 'info', summary: 'Already logged in', life: 3000 });
      if (window.history.length > 1) {
        router.back();
      } else {
        router.push('/');
      }
    }
  }).catch(error => {
    console.error('Error fetching user:', error);
    toast.add({ severity: 'error', summary: 'Error fetching user', life: 3000 });
  });
}

</script>


<template>
  <div>
    <h1>SimpleET</h1>
    <h2>{{ route.meta.name }}</h2>
    <br>
  </div>
  <router-view />

</template>
