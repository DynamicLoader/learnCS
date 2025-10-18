<script setup lang="ts">
import { ref, onMounted ,watch} from "vue";
import DataView from 'primevue/dataview';

const props = defineProps({
    items: {
        type: Array,
        required: true
    },
    itemComponent: {
        type: Object,
        required: true
    }
});

const localProducts = ref(props.items);

watch(() => props.items, (newProducts) => {
  localProducts.value = newProducts;
});

</script>

<template>
    <div class="card w-full">
        <DataView :value="localProducts" paginator :rows="5" class="w-full">
            <template #list="slotProps">
                <div class="flex flex-col w-full">
                    
          <component
            v-for="(item, index) in slotProps.items"
            :is="itemComponent"
            :key="index"
            :item="item"
            :index="index"
          />
                    <!-- <div v-for="(item, index) in slotProps.items" :key="index"></div> -->
                </div>
            </template>
        </DataView>
    </div>
</template>
