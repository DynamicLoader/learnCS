<script lang="ts" setup>
import CartItem from "@/components/CartItem.vue";
import ItemList from "@/components/ItemList.vue";
import Button from "primevue/button"
import InputText from "primevue/inputtext"
import { func } from "superstruct";
import { ref, onMounted, reactive } from 'vue';
import { Toast, useToast } from "primevue";


import { useRoute, useRouter } from 'vue-router';

const route = useRoute();
const router = useRouter();
const toast = useToast();

interface CartItem {
    pid: string;
    name: string;
    img: string;
    seller: string;
    status: string;
    price: number;
    count: number;
    totalPrice: number;
}

const cartitems = ref([]);

onMounted(async () => {
    fetch('/api/user/cart', {
        method: 'GET',
    })
        .then(response => response.json())
        .then(data => {
            console.log(data);
            if(data.code === 401) {
                router.push('/auth/login');
                return;
            }

            cartitems.value = data.data.map((item: CartItem) => ({
                pid: item.pid,
                link: `/product/${item.pid}`,
                name: item.name,
                seller: item.seller,
                image: item.img,
                inventoryStatus: 'INSTOCK',
                price: item.price.toFixed(2),
                totalPrice: (item.price * item.count).toFixed(2),
                count: item.count,
            }));
            console.log(cartitems);
        })
        .catch(error => {
            toast.add({ severity: 'error', summary: 'Error fetching cart items', life: 3000 });
        });
});

function navigateToLink(link: string) {
    console.log(link);
    router.push(link);
}

function clearCart() {
    fetch('/api/user/cart', {
        method: 'DELETE',
    })
        .then(response => response.json())
        .then(data => {
            if(data.code === 401) {
                router.push('/auth/login');
                return;
            }
            console.log('Cart cleared:', data);
            location.reload();
        })
        .catch(error => {
            console.error('Error clearing cart:', error);
            toast.add({ severity: 'error', summary: 'Error clearing cart', life: 3000 });
        });
}

</script>


<template>
    <div class="w-full">

        <div class="flex flex-row justify-between items-center mb-4">
            <h1>Cart</h1>
            <div class="flex gap-2">
                <Button label="Checkout" icon="pi pi-check" class="p-button-success flex flex-row md:items-end "
                    @click="navigateToLink('/buy/cart')" />
                <Button label="Clear Cart" icon="pi pi-trash" class="p-button-danger" @click="clearCart" />
            </div>
        </div>
        <ItemList :items="cartitems" :itemComponent="CartItem" class="w-full" />
    </div>
</template>
