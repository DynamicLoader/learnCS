package org.dyldr.SimpleET.Service.Order;

import jakarta.transaction.Transactional;
import org.dyldr.SimpleET.DB.OrderDetailTable;
import org.dyldr.SimpleET.DB.OrderTable;
import org.dyldr.SimpleET.DB.ProductTable;
import org.dyldr.SimpleET.Entity.OrderDetailEntity;
import org.dyldr.SimpleET.Entity.OrderEntity;
import org.dyldr.SimpleET.Entity.ProductEntity;
import org.dyldr.SimpleET.Entity.UserEntity;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;

import java.util.ArrayList;
import java.util.Date;
import java.util.List;
import java.util.Map;

@Service
public class OrderService {

    @Autowired
    private OrderTable orderTable;

    @Autowired
    private OrderDetailTable orderDetailTable;

    @Autowired
    private ProductTable productTable;


    public List<OrderEntity> getByUid(String uid) {
        UserEntity u = new UserEntity();
        u.setId(uid);
        return orderTable.findByCustomer(u);
    }

    public List<OrderEntity> getAll() {
        var ret = new ArrayList<OrderEntity>();
        orderTable.findAll().forEach(ret::add);
        return ret;
    }

    public OrderEntity getById(String oid) {
        return orderTable.findById(oid).orElse(null);
    }

    public List<OrderDetailEntity> getDetailByOid(String oid) {
        OrderEntity o = new OrderEntity();
        o.setId(oid);
        return orderDetailTable.findByOrder(o);
    }

    @Transactional
    public OrderEntity createOrder(String uid, String address, Map<String, Integer> detail) {
        UserEntity u = new UserEntity();
        u.setId(uid);
        OrderEntity o = new OrderEntity();
        o.setCustomer(u);
        o.setAddress(address);
        Date date = new Date(System.currentTimeMillis());
        o.setCreateTime(date);
        o.setUpdateTime(date);
        o.setStatus(OrderEntity.OrderStatus.CREATED);

        ArrayList<OrderDetailEntity> od = new ArrayList<>();
        for (Map.Entry<String, Integer> entry : detail.entrySet()) {
            OrderDetailEntity d = new OrderDetailEntity();
            ProductEntity pr = productTable.findById(entry.getKey()).orElse(null);
            if(pr.getStock() < entry.getValue())
                throw new RuntimeException("Stock not enough");
            pr.setStock(pr.getStock() - entry.getValue());
            pr.setTotalSold(pr.getTotalSold() + entry.getValue());

            d.setProduct(pr);
            d.setCount(entry.getValue());
            d.setPrice(pr.getPrice());
            d.setOrder(o);
            od.add(d);
            o.setCount(o.getCount() + d.getCount());
            o.setTotalPrice(o.getTotalPrice() + d.getTotalPrice());
        }

        orderTable.save(o); // Make sure the order is saved first
        orderDetailTable.saveAll(od);
        return o;
    }

    @Transactional
    public OrderEntity updateOrder(String oid, String address) {
        OrderEntity o = orderTable.findById(oid).orElse(null);
        if (o == null) {
            return null;
        }
        o.setAddress(address);
        o.setUpdateTime(new Date(System.currentTimeMillis()));
        orderTable.save(o);
        return o;
    }

    @Transactional
    public OrderEntity updateOrder(String oid, OrderEntity.OrderStatus status) {
        OrderEntity o = orderTable.findById(oid).orElse(null);
        if (o == null) {
            return null;
        }
//        o.setAddress(address);
        o.setStatus(status);
        o.setUpdateTime(new Date(System.currentTimeMillis()));
        orderTable.save(o);
        return o;
    }

    @Transactional
    public OrderEntity payOrderBuiltin(String oid, String payId) {
        OrderEntity o = orderTable.findById(oid).orElse(null);
        if (o == null) {
            return null;
        }
        o.setStatus(OrderEntity.OrderStatus.PAID);
        o.setUpdateTime(new Date(System.currentTimeMillis()));
        orderTable.save(o);
        return o;
    }

}
