<script lang="ts" setup>
import { Form, FormField } from '@primevue/forms';
import InputText from 'primevue/inputtext';
import Message from 'primevue/message';
import Button from 'primevue/button';
import Password from 'primevue/password';
import Textarea from 'primevue/textarea';
import { is, object, string } from 'superstruct';
import { superStructResolver } from '@primevue/forms/resolvers/superstruct';

import { nonempty, } from 'superstruct'

import { ref } from 'vue';
import { useRouter } from 'vue-router'
import Dialog from 'primevue/dialog';
import { error } from 'console';

const showDialog = ref(false);
const errorMessage = ref('');
const errorTitle = ref('Error');
const router = useRouter();

function onFormSubmit(v) {
    const { states, errors } = v;
    console.log(v);
    if (!v.valid) {
        console.log('Form is valid');
        return;
    }

    const formData = new FormData();
    formData.append('email', states.email.value);
    formData.append('username', states.username.value);
    formData.append('password', states.password.value);


    fetch('/api/auth/register', {
        method: 'POST',
        body: formData,
    })
        .then(response => response.json())
        .then(data => {
            console.log('Success:', data);
            if (data.code === 0) {
                errorTitle.value = 'Registeration Success';
                errorMessage.value = 'Redirecting to login page...';
                showDialog.value = true;
                setTimeout(() => {
                    showDialog.value = false;
                    router.replace('/auth/login');
                }, 2000);
            } else {
                errorMessage.value = data.msg[0] || 'Registration failed';
                showDialog.value = true;
            }
        })
        .catch(error => {
            console.error('Error:', error);
            errorMessage.value = 'An error occurred during registration';
            showDialog.value = true;
        });
}

const schema = object({
    email: nonempty(string()),
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
    <Form :initialValues :resolver @submit="onFormSubmit" class="flex flex-col gap-4 w-full sm:w-80">
        <FormField v-slot="$field" name="email" initialValue="" class="flex flex-col gap-1">
            <InputText type="text" placeholder="Email" />
            <Message v-if="$field?.invalid" severity="error" size="small" variant="simple">{{ $field.error?.message }}
            </Message>
        </FormField>
        <FormField v-slot="$field" name="username" initialValue="" class="flex flex-col gap-1">
            <InputText type="text" placeholder="Username" />
            <Message v-if="$field?.invalid" severity="error" size="small" variant="simple">{{ $field.error?.message }}
            </Message>
        </FormField>
        <FormField v-slot="$field" name="password" initialValue="" class="flex flex-col gap-1">
            <Password type="text" placeholder="Password" :feedback="false" toggleMask fluid />
            <Message v-if="$field?.invalid" severity="error" size="small" variant="simple">{{ $field.error?.message }}
            </Message>
        </FormField>
        <Button type="submit" severity="secondary" label="Register Now!" />
        <RouterLink to="/auth/login">Login with existing account</RouterLink>
    </Form>
    

    <Dialog v-model:visible="showDialog" modal v-model:header="errorTitle" :style="{ width: '25rem' }">
        <span class="text-surface-500 dark:text-surface-400 block mb-8">{{ errorMessage }}</span>
        <div class="flex justify-end gap-2">
            <Button type="button" label="OK" severity="secondary" @click="showDialog = false"></Button>
        </div>
    </Dialog>
</template>
