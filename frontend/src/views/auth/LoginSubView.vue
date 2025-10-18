<script lang="ts" setup>
import { Form, FormField } from '@primevue/forms';
import InputText from 'primevue/inputtext';
import Message from 'primevue/message';
import Button from 'primevue/button';
import Password from 'primevue/password';
import { object, string } from 'superstruct';
import { superStructResolver } from '@primevue/forms/resolvers/superstruct';
import { nonempty } from 'superstruct'

import { ref } from 'vue';
import { RouterLink, useRouter } from 'vue-router'
import Dialog from 'primevue/dialog';
import { useUserStore } from '@/stores/userStore';

const showDialog = ref(false);
const errorMessage = ref('');;
const router = useRouter();
const userStore = useUserStore();
const { username, uid, role, setUser, clearUser } = userStore;

function onFormSubmit(v) {
    const { states, errors } = v;
    console.log(v);
    if (!v.valid) {
        console.log('Form is invalid');
        return;
    }

    // console.log(globals.apiPrefix);
    const formData = new FormData();
    formData.append('username', states.username.value);
    formData.append('password', states.password.value);

    fetch('/api/auth/login', {
        method: 'POST',
        body: formData,
    })
        .then(response => response.json())
        .then(data => {
            console.log('Success:', data);
            if (data.code === 0) {
                setUser(states.username.value, data.msg[1], data.msg[2]);
                console.log('User:', userStore.username, userStore.uid, userStore.role);
                if (window.history.length > 1) {
                    router.back()
                } else {
                    router.push('/')
                }
            } else {
                errorMessage.value = data.msg[0] || 'Login failed';
                showDialog.value = true;
            }
        })
        .catch(error => {
            console.error('Error:', error);
            errorMessage.value = 'An error occurred during login';
            showDialog.value = true;
        });
}

const schema = object({
    username: nonempty(string()),
    password: nonempty(string()),
});

const initialValues = ref({
    username: '',
    password: '',
});

// Create the resolver using superstructResolver
const resolver = superStructResolver(schema);
</script>

<template>

    <Form v-slot="$form" :initialValues :resolver @submit="onFormSubmit" class="flex flex-col gap-4 w-full sm:w-80">
        <div class="flex flex-col gap-1">
            <InputText name="username" type="text" placeholder="Username" fluid />
            <Message v-if="$form.username?.invalid" severity="error" size="small" variant="simple">{{
                $form.username.error.message }}</Message>
        </div>
        <div class="flex flex-col gap-1">
            <Password name="password" placeholder="Password" :feedback="false" toggleMask fluid />
            <Message v-if="$form.password?.invalid" severity="error" size="small" variant="simple">
                <ul class="my-0 px-4 flex flex-col gap-1">
                    <li v-for="(error, index) of $form.password.errors" :key="index">{{ error.message }}</li>
                </ul>
            </Message>
        </div>
        <Button type="submit" severity="secondary" label="Login" />
        <RouterLink to="/auth/register">Don't have an account? Register a new one!</RouterLink>
    </Form>

    <Dialog v-model:visible="showDialog" modal header="Error" :style="{ width: '25rem' }">
        <span class="text-surface-500 dark:text-surface-400 block mb-8">{{ errorMessage }}</span>
        <div class="flex justify-end gap-2">
            <Button type="button" label="OK" severity="secondary" @click="showDialog = false"></Button>
        </div>
    </Dialog>

</template>
