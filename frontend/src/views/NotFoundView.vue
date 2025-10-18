<template>
  <div class="not-found">
    <h1>404 - Page Not Found</h1>
    <p>The page you are looking for does not exist.</p>
    <router-link to="/">Go to Home</router-link>
  </div>
</template>

<script lang="ts" setup>
</script>

<style scoped>
.not-found {
  text-align: center;
  margin-top: 50px;
}
</style>