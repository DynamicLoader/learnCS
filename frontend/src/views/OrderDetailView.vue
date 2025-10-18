<template>
    <div class="w-full">
        <div v-if="!notDetail" class="flex flex-row justify-between items-center mb-4">
            <h1>Order Details</h1>
            <div class="flex gap-2">
                <Button :label="!orderHasConfirmed ? 'Confirm Receipt' : 'Finished'" icon="pi pi-check"
                    class="p-button-success flex flex-row md:items-end" :disabled="orderHasConfirmed" @click="confirmOrder"/>
                <!-- <Button label="Cancel" icon="pi pi-trash" class="p-button-danger" @click="goback" /> -->
            </div>
        </div>

        <div>
            <h3>Customer: {{ customer }}</h3>
            <h3>Order ID: {{ orderID }}</h3>
            <h3>Total Items: {{ orderItemCount }}</h3>
            <h3>Total Price: CNY {{ orderTotalPrice.toFixed(2) }}</h3>
            <h3>Address: {{ orderAddress }}</h3>
            <h3>Created: {{ createdTime }}</h3>
            <h3>Status: {{ orderHasConfirmed ? 'FINISHED' : 'PENDING' }}</h3>
            <ItemList :items="orderItems" :item-component="BuyItem" />
        </div>
    </div>
</template>


<script lang="ts" setup>

import { defineProps } from 'vue';
import { onMounted } from 'vue';
import { useRoute, useRouter } from 'vue-router';
import { ref, watch } from 'vue';


import Button from 'primevue/button';
import ItemList from '@/components/ItemList.vue';
import BuyItem from '@/components/BuyItem.vue';
import { useToast } from 'primevue';

const toast = useToast();
const route = useRoute();
const router = useRouter();
const title = ref('Order View');
const isOrderDetail = ref(false);
const orderID = ref('');
const orderItemCount = ref(0);
const orderTotalPrice = ref(0);
const orderAddress = ref('');
const orderItems = ref([]);
const orderHasConfirmed = ref(false);
const customer = ref('');
const createdTime = ref('');

const props = defineProps({
    orderid: {
        type: String,
        required: true
    },
    notDetail: {
        type: Boolean,
        required: false,
        default: false
    }
});

function update() {
    const newId = props.orderid;
    orderID.value = newId.toString();

    const fetchOrder = async (id: string) => {
        try {
            const response = await fetch(`/api/order/${id}`);
            const result = await response.json();
            console.log(result);

            if (result.code === 0) {
                orderItemCount.value = result.data.count;
                orderTotalPrice.value = result.data.total;
                orderItems.value = result.data.data;
                orderAddress.value = result.data.address;
                customer.value = result.data.customer;
                createdTime.value = result.data.created;
                if (result.data.status === 'FINISHED') {
                    orderHasConfirmed.value = true;
                } else {
                    orderHasConfirmed.value = false;
                }

            } else {
                console.error('Failed to fetch order:', result.message);
                toast.add({ severity: 'error', summary: 'Failed to fetch order', detail: result.message, life: 3000 });
            }
        } catch (e) {
            console.error('Failed to fetch order:', e);
            toast.add({ severity: 'error', summary: 'Failed to fetch order', detail: e, life: 3000 });
        };
    }

    fetchOrder(newId.toString());
}

function confirmOrder(){
    const formData = new FormData();
    formData.append('state', "FINISHED");

    fetch(`/api/order/${props.orderid}`, {
        method: 'POST',
        body: formData,
    })
        .then(response => response.json())
        .then(data => {
            if(data.code === 401) {
                router.push('/auth/login');
                return;
            }
            if(data.code !== 0) {
                console.error('Error confirming order:', data);
                toast.add({ severity: 'error', summary: 'Error confirming order', detail: data, life: 3000 });
                return;
            }
            console.log('Order confirmed:', data);
            toast.add({ severity: 'success', summary: 'Order confirmed', life: 3000 });
            orderHasConfirmed.value = true;
        })
        .catch(error => {
            console.error('Error confirming order:', error);
            toast.add({ severity: 'error', summary: 'Error confirming order', detail: error, life: 3000 });
        });
}

onMounted(
    () => {
        // console.log('OrderDetailView mounted');
        // const newId = route.params.id;
        update();
    }
)

watch(
    () => props.orderid,
    (newId) => {
        if (newId) {
            // console.log('OrderDetailView watch:', newId);
            update();
        }
    },
    { immediate: true }
);

</script>
