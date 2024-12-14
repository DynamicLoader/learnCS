<script setup>
const props = defineProps({
    author: Object,
    ranking: Number
});

const rank = ['FIRST', 'SECOND', 'THIRD']
</script>


<template>
    <div
        class="border-solid border-gray-300 rounded-md border-[1px] p-6 my-3 shadow-lg hover:shadow-xl transition-shadow duration-200">
        <p class="text-sm text-gray-400 mb-2">作者</p>
        <p class="text-2xl font-bold my-4 text-blue-500">{{ props.author.name }}</p>
        <div class="flex justify-between items-center mt-4">
            <p class="text-lg text-gray-500">{{ props.author.workcount }} 篇作品</p>
            <p v-if="ranking <= 100" class="font-semibold">
                THE
                <span :class="{
                    'text-gold': ranking === 1,
                    'text-silver': ranking === 2,
                    'text-bronze': ranking === 3
                }" style="font-size: 2em; font-weight: bold;">{{ ranking }}</span>
                OF <router-link :to="{ path: '/stats', query: props.author.name }"
                    class="text-blue-500 hover:text-blue-700 transition-colors duration-200">TOP100AUTHORS</router-link>
            </p>
        </div>
    </div>
</template>

<style scoped>
.text-gold {
    color: rgba(255, 217, 0, 0.849);
}

.text-silver {
    color: silver;
}

.text-bronze {
    color: #cd7f32;
}
</style>