package org.dyldr.SimpleET.WebAPI.User;


import org.dyldr.SimpleET.Entity.ProductEntity;
import org.dyldr.SimpleET.Service.Product.ProductListService;
import org.dyldr.SimpleET.Service.User.CartService;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Controller;
import org.springframework.web.bind.annotation.*;

import java.util.ArrayList;
import java.util.List;
import java.util.Map;

@RestController
@RequestMapping("/user/cart")
public class CartController {

    @Autowired
    private CartService cartService;

    @Autowired
    private ProductListService prdListSrv;

    record CartItem(String pid, String name, String img, String seller, String status, double price, int count, double totalPrice) {
    }

    record CartRet<T>(int code, T data) {
    }

    @PostMapping("")
    public CartRet<String> addProductToCart(
            @RequestAttribute(value = "token") Map<String, String> token,
            @RequestParam(value = "pid") String pid,
            @RequestParam(value = "count", defaultValue = "1") int count
    ) {
        try {
            String uid = token.get("uid");
            cartService.addProductToCart(uid, pid, count);
            return new CartRet<String>(0, "Add to cart successful");
        } catch (Exception e) {
            return new CartRet<String>(-1, "Add to cart failed");
        }
    }

    @DeleteMapping("")
    public CartRet<String> clearCart(
            @RequestAttribute (value = "token") Map<String, String> token
    ){
        try {
            String uid = token.get("uid");
            cartService.clearCart(uid);
            return new CartRet<String>(0, "Clear cart successful");
        } catch (Exception e) {
            return new CartRet<String>(-1, "Clear cart failed");
        }
    }

    @GetMapping("")
    public CartRet<List<CartItem>> getCart(
            @RequestAttribute(value = "token") Map<String, String> token
    ) {
        try {
            String uid = token.get("uid");
            Map<String, Integer> cart = cartService.getCart(uid);
            List<CartItem> ret = new ArrayList<CartItem>(cart.size());
            for (Map.Entry<String, Integer> e : cart.entrySet()) {
                ProductEntity p = prdListSrv.getProduct(e.getKey());
                ret.add(new CartItem(e.getKey(), p.getName(),p.getImage() , p.getSeller().getName(),p.getStatus().toString(),p.getPrice(), e.getValue(), p.getPrice() * e.getValue()));
            }
            return new CartRet<List<CartItem>>(0, ret);
        } catch (Exception e) {
            return new CartRet<List<CartItem>>(0, null);
        }
    }

    @PostMapping("{pid}")
    public CartRet<String> updateProductCount(
            @RequestAttribute(value = "token") Map<String, String> token,
            @PathVariable(value = "pid") String pid,
            @RequestParam(value = "count") int count
    ) {
        try {
            String uid = token.get("uid");
            cartService.updateProductCount(uid, pid, count);
            return new CartRet<String>(0, "Update count successful");
        } catch (Exception e) {
            return new CartRet<String>(-1, "Update count failed");
        }
    }

    @DeleteMapping("{pid}")
    public CartRet<String> removeProductFromCart(
            @RequestAttribute(value = "token") Map<String, String> token,
            @PathVariable(value = "pid") String pid
    ) {
        try {
            String uid = token.get("uid");
            cartService.removeProductFromCart(uid, pid);
            return new CartRet<String>(0, "Remove from cart successful");
        } catch (Exception e) {
            return new CartRet<String>(-1, "Remove from cart failed");
        }
    }

}
