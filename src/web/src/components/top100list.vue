<script setup>
import { defineProps } from "vue";
const props = defineProps({
    top100: Object,
    ty: String,
});
</script>

<template>
    <ol class="ml-10space-y-2">
        <div
            v-for="(item, index) in top100"
            :key="index"
            class="text-gray-700 text-lg text-center font-semibold bg-gray-100 py-3 px-4 my-2 rounded-sm shadow-sm relative overflow-hidden"
        >
            <div v-if="ty === 'author'">
                <router-link
                    :to="{
                        path: '/author',
                        query: { author: item },
                    }"
                >
                    {{ item }}
                </router-link>
            </div>
            <div v-if="ty === 'keyword'">
                <router-link
                    :to="{
                        path: '/s',
                        query: { query: item },
                    }"
                >
                    {{ item }}
                </router-link>
            </div>

            <span
                class="absolute bottom-[1px] left-[1px] text-[64px] font-bold opacity-15"
                >{{ index + 1 }}</span
            >
        </div>
    </ol>
</template>

<style scoped>
/* Your component styles go here */
</style>
