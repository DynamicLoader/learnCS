<template>
    <div class="flex flex-col sm:flex-row sm:items-center p-6 gap-4"
        :class="{ 'border-t border-surface-200 dark:border-surface-700': index !== 0 }">
        <div  class="md:w-40 relative">
            <img style="width:100%" class="block xl:block mx-auto rounded " :src="item.img" :alt="item.name" />
            <!-- <div class="absolute bg-black/70 rounded-border" style="left: 4px; top: 4px">
                <Tag :value="item.inventoryStatus" :severity="item.severity"></Tag>
            </div> -->
        </div>
        <div class="flex flex-col md:flex-row justify-between md:items-center flex-1 gap-6">
            <div  class="flex flex-row md:flex-col justify-between items-start gap-2">
                <div>
                    <span class="font-medium text-surface-500 dark:text-surface-400 text-sm">{{ item.seller }}</span>
                    <div class="text-lg font-medium mt-2">{{ item.name }}</div>
                </div>
                <div class="bg-surface-100 p-1" style="border-radius: 30px">
                </div>
            </div>
            <div class="flex flex-col md:items-end gap-8">
                <div class="flex flex-row-reverse md:flex-row gap-2">
                    <span class="text-xl">{{ item.count }}</span>
                    <span> x </span>
                    <span class="text-xl">CNY {{ item.price.toFixed(2) }}</span>
                </div>
                <div class="flex flex-row-reverse md:flex-row gap-2">
                    
                    <span class="text-xl font-semibold">Total: CNY {{ item.totalPrice.toFixed(2) }}</span>
                </div>

            </div>
        </div>
    </div>
</template>


<script setup lang="ts">
import { defineProps, ref, watch } from 'vue';
import Tag from 'primevue/tag';
import Button from 'primevue/button';
import InputNumber from 'primevue/inputnumber';
import router from '@/router';

const props = defineProps({
    item: {
        type: Object,
        required: true
    },
    index: {
        type: Number,
        required: true
    }
});

</script>