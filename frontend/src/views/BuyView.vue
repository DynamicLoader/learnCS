<!--
Order Detail page
-->

<script lang="ts" setup>
import BuyItem from '@/components/BuyItem.vue';
import ItemList from '@/components/ItemList.vue';
import Button from 'primevue/button';
import { func } from 'superstruct';
import { defineComponent, ref, onMounted } from 'vue';
import { useRoute, useRouter } from 'vue-router';
import { InputGroup, InputText, FloatLabel } from 'primevue';
import { useToast } from 'primevue';
import { useConfirm } from 'primevue';

const toast = useToast();
const confirm = useConfirm();
const route = useRoute();
const router = useRouter();
const address = ref('');
const cartItems = ref([]);
const itemCount = ref(1);


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

const createOrder = async (detail: string) => {
    console.log(detail);
    try {
        const response = await fetch('/api/order', {
            method: 'POST',
            headers: {
                'Content-Type': 'application/x-www-form-urlencoded',
            },
            body: new URLSearchParams({
                address: address.value,
                detail: detail,
            }),
        });
        const result = await response.json();
        console.log(result);
        if (result.code === 0) {
            // alert('Order created successfully!');
            toast.add({ severity: 'success', summary: 'Order created successfully!', detail: "An email will be sent when your order is OK, please mind!"});
            router.replace(`/order/${result.data}`);
        } else {
            console.error('Failed to create order: ' + result.message);
            toast.add({ severity: 'error', summary: 'Failed to create order: ' + result.message, life: 3000 });
        }
    } catch (e) {
        console.error('Failed to create order: ' + e);
        toast.add({ severity: 'error', summary: 'Failed to create order: ' + e, life: 3000 });
    };
}

const handleCheckout = () => {
    // console.log(cartItems.value);

    const detailMap = new Map();
    cartItems.value.forEach((item: CartItem) => {
        detailMap.set(item.pid, item.count);
    });
    console.log(detailMap);
    const detailObject = Object.fromEntries(detailMap);
    createOrder(JSON.stringify(detailObject));
};

const handleSinglePurchase = () => {
    const itemId = route.params.id.toString();
    const detail = JSON.stringify({ [itemId]: itemCount.value });
    createOrder(detail);
};

function handleProcess() {

    if (route.path === '/buy/cart') {
        handleCheckout();
    } else if (route.path.startsWith('/buy/single/')) {
        handleSinglePurchase();
    }
}

onMounted(() => {
    if (route.path === '/buy/cart') {
        const fetchCartItems = async () => {
            const response = await fetch('/api/user/cart');
            const ret = await response.json();
            if (ret.code === 0) {
                cartItems.value = ret.data;
            }
            console.log(cartItems.value);
        };

        fetchCartItems();
    } else if (route.path.startsWith('/buy/single/')) {
        itemCount.value = Number(route.query.count) || 1;
        const fetchProduct = async (id: string) => {
            const response = await fetch(`/api/product/${id}`);
            const result = await response.json();
            console.log(result);
            if (result.code === 0) {
                const product = result.data;
                const cartItem: CartItem = {
                    pid: product.id,
                    name: product.name,
                    img: product.image,
                    seller: product.seller,
                    status: 'available',
                    price: product.price,
                    count: itemCount.value,
                    totalPrice: product.price * itemCount.value,
                };
                cartItems.value = [cartItem];
            } else {
                console.error('Failed to fetch product:', result.message);
            }
        };
        fetchProduct(route.params.id.toString());
    }
});

function goback() {
    if (window.history.length > 1) {
        router.back();
    } else {
        router.push('/');
    }
}

const confirmDialog = () => {
    if (address.value === '') {
        toast.add({ severity: 'error', summary: 'Address is required!', life: 3000 });
        return;
    }

    confirm.require({
        message: 'Are you sure you want to proceed? \nThis will create an order and PAY FOR IT FROM YOUR WALLET!\nPlease also make sure your address is correct: \n' + address.value,
        header: 'Confirmation',
        icon: 'pi pi-exclamation-triangle',
        rejectProps: {
            label: 'Cancel',
            severity: 'secondary',
            outlined: true
        },
        accept: () => {
            handleProcess();
        },
        // reject: () => {
        //     toast.add({ severity: 'warn', summary: 'Cancelled' });
        //     goback();
        // }
    });
};

</script>

<template>
    <div class="w-full">
        <div class="flex flex-row justify-between items-center mb-4">
            <h1>Order Confirm</h1>
            <div class="flex gap-2">
                <Button label="Process (Auto Pay)" icon="pi pi-check"
                    class="p-button-success flex flex-row md:items-end " @click="confirmDialog" />
                <Button label="Cancel" icon="pi pi-trash" class="p-button-danger" @click="goback" />
            </div>
        </div>
        <br>
        <InputGroup>
            <FloatLabel variant="on">
                <InputText id="category" v-model="address" />
                <label for="category">Address</label>
            </FloatLabel>
        </InputGroup>
        <br>
        <h2> Order Summary: {{ cartItems.reduce((acc, item) => acc + item.count, 0) }} Item(s), CNY {{
            cartItems.reduce((acc, item) => acc + item.totalPrice, 0).toFixed(2) }}</h2>
        <ItemList :items="cartItems" :item-component="BuyItem" />
    </div>

</template>