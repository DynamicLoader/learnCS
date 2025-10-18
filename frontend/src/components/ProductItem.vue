<template>
  <div class="flex flex-col sm:flex-row sm:items-center p-6 gap-4"
    :class="{ 'border-t border-surface-200 dark:border-surface-700': index !== 0 }">
    <div @click="navigateToLink(item.link)" class="md:w-40 relative">
      <img style="width: 100%;" class="block xl:block mx-auto rounded w-full" :src="item.image" :alt="item.name" />
      <div class="absolute bg-black/70 rounded-border" style="left: 4px; top: 4px">
        <Tag :value="item.inventoryStatus" :severity="item.severity"></Tag>
      </div>
    </div>
    <div class="flex flex-col md:flex-row justify-between md:items-center flex-1 gap-6">
      <div @click="navigateToLink(item.link)" class="flex flex-row md:flex-col justify-between items-start gap-2">
        <div>
          <span class="font-medium text-surface-500 dark:text-surface-400 text-sm">{{ item.category }}</span>
          <div class="text-lg font-medium mt-2">{{ item.name }}</div>
        </div>
        <div class="bg-surface-100 p-1" style="border-radius: 30px">
          <div class="bg-surface-0 flex items-center gap-2 justify-center py-1 px-2" style="border-radius: 30px; box-shadow: 0px 1px 2px 0px rgba(0, 0, 0, 0.04), 0px 1px 2px 0px rgba(0, 0, 0, 0.06)">
          <i class="pi pi-star-fill text-yellow-500"></i>
          <span class="text-surface-900 font-medium text-sm">{{ item.totalSold }}</span>
          </div>
        </div>
      </div>
      <div class="flex flex-col md:items-end gap-8">
        <span class="text-xl font-semibold">{{ item.price }}</span>
        <div class="flex flex-row-reverse md:flex-row gap-2" v-if="canBuy === true">
          <Button icon="pi pi-plus" outlined label="Add to Cart" @click="addToCart"></Button>
          <Button icon="pi pi-shopping-cart" label="Buy Now" @click="navigateToLink(item.primaryLink)"
            :disabled="item.inventoryStatus === 'OUTOFSTOCK'"
            class="flex-auto md:flex-initial whitespace-nowrap"></Button>
        </div>
      </div>
    </div>
  </div>
</template>

<script setup lang="ts">
import { defineProps, onMounted, ref, watch } from 'vue';
import Tag from 'primevue/tag';
import Button from 'primevue/button';
import router from '@/router';
import { useToast } from 'primevue';
import { useUserStore } from '@/stores/userStore';

const toast = useToast();
const userStore = useUserStore();
const canBuy = ref(false);

const updateParam = () => {
  canBuy.value = userStore.role === 'CUSTOMER';
};

watch(() => userStore.role, () => {
  updateParam();
});

onMounted(() => {
  updateParam();
});

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

function navigateToLink(link: string) {
  console.log(link);
  router.push(link);
}

const addToCart = async () => {
  var f = new FormData;
  f.append('pid', props.item.pid);
  f.append('count', "1");

  await fetch('/api/user/cart', {
    method: 'POST',
    body: f
  })
    .then(response => response.json())
    .then(data => {
      if (data.code === 401) {
        router.push('/auth/login');
        return;
      }

      if (data.code === 0) {
        // console.log('Item added to cart:', data);
        toast.add({ severity: 'success', summary: 'Product added to Cart', life: 3000 });
      } else {
        console.error('Error adding item to cart:', data);
        toast.add({ severity: 'error', summary: 'Error adding item to Cart' + data.data, life: 3000 });
      }
    })
    .catch(error => {
      console.error('Error adding item to cart:', error);
      toast.add({ severity: 'error', summary: 'Error adding item to Cart' + error, life: 3000 });
    });
}

</script>