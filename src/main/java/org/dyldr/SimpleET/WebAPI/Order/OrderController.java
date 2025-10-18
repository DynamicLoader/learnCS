package org.dyldr.SimpleET.WebAPI.Order;


import com.fasterxml.jackson.core.type.TypeReference;
import org.dyldr.SimpleET.Entity.OrderDetailEntity;
import org.dyldr.SimpleET.Entity.OrderEntity;
import org.dyldr.SimpleET.Service.Order.OrderService;
import org.dyldr.SimpleET.Util.EmailUtil;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.web.bind.annotation.*;
import com.fasterxml.jackson.databind.ObjectMapper;

import java.util.Date;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

@RestController
@RequestMapping("/order")
public class OrderController {

    @Autowired
    private OrderService orderService;

    record OrderRet<T>(int code, T data) {
    }

    record OrderItem(String oid, String customer, OrderEntity.OrderStatus status, int count, double total, Date created) {
    }

    record OrderDetailItem(String pid, int count, double price, double totalPrice, String img, String name,
                           String seller) {
    }

    record OrderDetailRet(String oid, OrderEntity.OrderStatus status, int count, double total, Date created, String address,String customer,
                          List<OrderDetailItem> data) {
    }

    @GetMapping("")
    public OrderRet<List<OrderItem>> getAll(
            @RequestAttribute(value = "token") Map<String, String> token
    ) {
        try {
            List<OrderEntity> orders;
            if (token.get("role").equals("ADMIN")) {
                orders = orderService.getAll();
            } else {
                orders = orderService.getByUid(token.get("uid"));
            }
//            orderService.getByUid(token.get("uid"));
            List<OrderItem> ret = orders.stream().map(
                    o -> new OrderItem(
                            o.getId(),
                            o.getCustomer().getName(),
                            o.getStatus(),
                            o.getCount(),
                            o.getTotalPrice(),
                            o.getCreateTime()
                    )
            ).toList();
            return new OrderRet<>(0, ret);
        } catch (Exception e) {
            return new OrderRet<>(-1, null);
        }
    }

    Map<String, Integer> jsonToMap(String json) throws Exception {
        ObjectMapper mapper = new ObjectMapper();
        Map<String, Integer> map = new HashMap<>();


        // convert JSON string to Map
        map = mapper.readValue(json, new TypeReference<Map<String, Integer>>() {
        });

        return map;
    }

    @PostMapping("")
    public OrderRet<String> createOrder(
            @RequestAttribute(value = "token") Map<String, String> token,
            @RequestParam(value = "address") String addr,
            @RequestParam(value = "detail") String det
    ) {
        try {
            var od = orderService.createOrder(token.get("uid"), addr, jsonToMap(det));
//            Send email
//            var emailUtil = new EmailUtil();
//            emailUtil.sendEmail(od.getCustomer().getEmail(), "Order Created", "Your order has been created! Order ID: " + od.getId());
            return new OrderRet<>(0, od.getId());
        } catch (Exception e) {
            return new OrderRet<>(-1, "Error creating order");
        }
    }

    @GetMapping("/{oid}")
    public OrderRet<OrderDetailRet> getOrder(
            @PathVariable String oid,
            @RequestAttribute Map<String, String> token
    ) {
        try {
            OrderEntity oe = orderService.getById(oid);
            if (oe == null) {
                return new OrderRet<>(-1, null);
            }
            if (!oe.getCustomer().getId().equals(token.get("uid")) && !token.get("role").equals("ADMIN")) {
                return new OrderRet<>(-2, null);
            }
            List<OrderDetailEntity> odes = orderService.getDetailByOid(oid);
            if (odes == null) {
                return new OrderRet<>(-3, null);
            }
            return new OrderRet<>(0, new OrderDetailRet(
                    oid,
                    oe.getStatus(),
                    oe.getCount(),
                    oe.getTotalPrice(),
                    oe.getCreateTime(),
                    oe.getAddress(),
                    oe.getCustomer().getName(),

                    odes.stream().map(
                            d -> new OrderDetailItem(
                                    d.getProduct().getId(),
                                    d.getCount(),
                                    d.getPrice(),
                                    d.getTotalPrice(),
                                    d.getProduct().getImage(),
                                    d.getProduct().getName(),
                                    d.getProduct().getSeller().getName()
                            )
                    ).toList()
            ));
        } catch (Exception e) {
            return new OrderRet<>(-99, null);
        }
    }

    @PutMapping("/{oid}")
    public OrderRet<String> updateOrder(
            @PathVariable String oid,
            @RequestParam(value = "address") String address,
            @RequestAttribute Map<String, String> token
    ) {
        try {
            OrderEntity oe = orderService.getById(oid);
            if (oe == null) {
                return new OrderRet<>(-1, null);
            }
            if (!oe.getCustomer().getId().equals(token.get("uid"))) {
                return new OrderRet<>(-2, null);
            }
            return new OrderRet<>(0, orderService.updateOrder(oid, address).getId());
        } catch (Exception e) {
            return new OrderRet<>(-99, null);
        }
    }

    @PostMapping("{oid}")
    public OrderRet<String> updateOrderState(
            @PathVariable String oid,
            @RequestParam(value = "state") OrderEntity.OrderStatus state,
            @RequestAttribute Map<String, String> token
    ) {
        try {
            OrderEntity oe = orderService.getById(oid);
            if (oe == null) {
                return new OrderRet<>(-1, null);
            }
            if (!oe.getCustomer().getId().equals(token.get("uid"))) {
                return new OrderRet<>(-2, null);
            }
            return new OrderRet<>(0, orderService.updateOrder(oid, state).getId());
        } catch (Exception e) {
            return new OrderRet<>(-99, null);
        }
    }

}
