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
                    <span class="font-medium text-surface-500 dark:text-surface-400 text-sm">{{ item.seller }}</span>
                    <div class="text-lg font-medium mt-2">{{ item.name }}</div>
                </div>
                <div class="bg-surface-100 p-1" style="border-radius: 30px">
                    <!-- <div class="bg-surface-0 flex items-center gap-2 justify-center py-1 px-2" style="border-radius: 30px; box-shadow: 0px 1px 2px 0px rgba(0, 0, 0, 0.04), 0px 1px 2px 0px rgba(0, 0, 0, 0.06)"> -->
                    <!-- <i class="pi pi-star-fill text-yellow-500"></i> -->
                    <!-- <span class="text-surface-900 font-medium text-sm">{{ item.rating }}</span> -->
                    <!-- </div> -->
                </div>
            </div>
            <div class="flex flex-col md:items-end gap-8">
                <div class="flex flex-row-reverse md:flex-row gap-2">
                <span class="text-xl">CNY {{ item.price }}</span>
                <span class="text-xl font-semibold">Total: {{ item.totalPrice }}</span>
                </div>
                <div class="flex flex-row-reverse md:flex-row gap-2">
                    <!-- <Button icon="pi pi-plus" outlined label="Add to Cart" @click="navigateToLink(item.secondaryLink)"></Button>
            <Button icon="pi pi-shopping-cart" label="Buy Now" @click="navigateToLink(item.primaryLink)" :disabled="item.inventoryStatus === 'OUTOFSTOCK'" class="flex-auto md:flex-initial whitespace-nowrap"></Button> -->
                    <InputNumber style="max-width: 150px;" @value-change="" v-model="itemcount"
                        inputId="horizontal-buttons" showButtons buttonLayout="horizontal" :step="1" fluid>
                        <template #incrementicon>
                            <span class="pi pi-plus" />
                        </template>
                        <template #decrementicon>
                            <span class="pi pi-minus" />
                        </template>
                    </InputNumber>
                    <Button icon="pi pi-trash" outlined label="Delete" @click="deleteCartItem()" class="p-button-danger"></Button>
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
import { useToast } from 'primevue';

const toast = useToast();

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

const itemcount = ref(props.item.count);

function deleteCartItem(){
    console.log("delete");

    fetch(`/api/user/cart/${props.item.pid}`, {
        method: 'DELETE',
    })
        .then(response => response.json())
        .then(data => {
            if(data.code === 401) {
                router.push('/auth/login');
                return;
            }
            console.log('Item deleted from cart:', data);
            location.reload();
        })
        .catch(error => {
            // console.error('Error deleting item from cart:', error);
            toast.add({ severity: 'error', summary: 'Error deleting item from Cart', life: 3000 });
        });
}

function updateCartItemCount() {
    const formData = new FormData();
    formData.append('count', itemcount.value.toString());

    fetch(`/api/user/cart/${props.item.pid}`, {
        method: 'POST',
        body: formData,
    })
        .then(response => response.json())
        .then(data => {
            if(data.code === 401) {
                router.push('/auth/login');
                return;
            }
            console.log('Item count updated:', data);
            props.item.totalPrice = (itemcount.value * props.item.price).toFixed(2);
        })
        .catch(error => {
            console.error('Error updating item count:', error);
            toast.add({ severity: 'error', summary: 'Error updating item count', life: 3000 });
        });
}

watch(() => itemcount.value, (newVal, oldVal) => {
    // console.log('itemcount changed from', oldVal, 'to', newVal);
    updateCartItemCount();
});

</script>