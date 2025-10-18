<template>
    <div class="w-full">
        <div class="flex flex-row justify-between items-center mb-4">
            <h1>Orders ({{ totalRecords }} in total)</h1>

            <div class="flex gap-2">
                <!-- <Button label="Process (Auto Pay)" icon="pi pi-check"
                    class="p-button-success flex flex-row md:items-end " @click="handleProcess" />
                <Button label="Cancel" icon="pi pi-trash" class="p-button-danger" @click="goback" /> -->
                <Paginator :rows="1" :totalRecords="totalRecords" @page="pageUpdate" :first="page">
                    <!-- <template #start="slotProps">
                        Total: 
                    </template> -->
                </Paginator>
            </div>
        </div>
        <hr>
        <div @click="navigatetoDetail">
            <component v-if="currentOrderId" :is="OrderDetailView" :orderid="currentOrderId" :notDetail="true"  />
        </div>
    </div>
</template>

<script lang="ts" setup>
import Paginator from 'primevue/paginator';
import { func } from 'superstruct';
import { onMounted, ref } from 'vue';
import OrderDetailView from '../views/OrderDetailView.vue';
import { useToast } from 'primevue';

const orderItems = ref([]);
import { useRoute } from 'vue-router';
import router from '@/router';


const route = useRoute();
const totalRecords = ref(0);
const currentOrderId = ref(null);
const toast = useToast();
const page = ref(0);

const fetchOrders = async () => {
    await fetch('/api/order').then(response => response.json()).then(data => {
        console.log(data);
        if (data.code === 401) {
            router.push('/auth/login');
            console.log("HERE?");
            return;
        }
        orderItems.value = data.data.sort((a: any, b: any) => new Date(b.created).getTime() - new Date(a.created).getTime());
        totalRecords.value = orderItems.value.length;
        console.log(orderItems);
    }).catch(error => {
        console.error('Error fetching orders:', error);
        toast.add({ severity: 'error', summary: 'Error fetching orders', life: 3000 });
    });
};


onMounted(() => {
    fetchOrders().then(() => {
        // console.log('Orders fetched', route.query.page);
        if(orderItems.value.length > 0) {
            pageUpdate({ page: route.query.page || 0 });
        }
        // pageUpdate({ page: route.query.page || 0 });
    });
    
});

function pageUpdate(event: any) {
    console.log(event.page);
    router.replace({ query: { page: event.page } });
    page.value = event.page;
    const orderId = orderItems.value[event.page].oid;
    console.log('orderId:', orderId);
    currentOrderId.value = orderId;
}

function navigatetoDetail() {
    console.log('navigatetoDetail');
    if (currentOrderId.value) {
        console.log('navigatetoDetail:', currentOrderId.value);
        router.replace({ query: { page: page.value } });
        router.push(`/order/${currentOrderId.value}`);
    }
}

</script>