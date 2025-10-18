package org.dyldr.SimpleET.Service.User;

import jakarta.transaction.Transactional;
import org.dyldr.SimpleET.DB.UserTable;
import org.dyldr.SimpleET.Entity.UserEntity;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;

import java.util.List;
import java.util.Map;

@Service
public class CartService {

    @Autowired
    private UserTable userTable;

    @Transactional
    public void addProductToCart(String uid, String pid, int count) {
        UserEntity ue =  userTable.findById(uid).orElse(null);
        if(ue == null) {
            throw new RuntimeException("User not found");
        }
        Map<String,Integer> cart = ue.getCart();
        if(cart == null)
            cart = new java.util.HashMap<>();
        if(cart.containsKey(pid)) {
            if(cart.get(pid) + count <= 0)
                throw new RuntimeException("Invalid count");
            cart.put(pid, cart.get(pid) + count);
        }else {
            if(count <= 0)
                throw new RuntimeException("Invalid count");
            cart.put(pid, count);
        }
        ue.setCart(cart);
        userTable.save(ue);
    }

    public void removeProductFromCart(String uid,String pid) {
        UserEntity ue =  userTable.findById(uid).orElse(null);
        if(ue == null) {
            throw new RuntimeException("User not found");
        }
        Map<String,Integer> cart = ue.getCart();
        if(cart == null)
            throw new RuntimeException("Cart is empty");
        if(cart.containsKey(pid)) {
            cart.remove(pid);
            ue.setCart(cart);
            userTable.save(ue);
            return;
        }
        throw new RuntimeException("Product not found in cart");
    }

    public void updateProductCount(String uid, String pid, int count) {
        UserEntity ue =  userTable.findById(uid).orElse(null);
        if(ue == null) {
            throw new RuntimeException("User not found");
        }
        Map<String,Integer> cart = ue.getCart();
        if(cart == null)
            throw new RuntimeException("Cart is empty");
        if(cart.containsKey(pid)) {
            if(count <= 0)
                throw new RuntimeException("Invalid count");
            cart.put(pid, count);
            ue.setCart(cart);
            userTable.save(ue);
            return;
        }
        throw new RuntimeException("Product not found in cart");
    }

    public void clearCart(String uid) {
        UserEntity ue =  userTable.findById(uid).orElse(null);
        if(ue == null) {
            throw new RuntimeException("User not found");
        }
        ue.setCart(new java.util.HashMap<>());
        userTable.save(ue);
    }

    public Map<String,Integer> getCart(String uid) {
        UserEntity ue =  userTable.findById(uid).orElse(null);
        if(ue == null) {
            throw new RuntimeException("User not found");
        }
        return ue.getCart();
    }


}
