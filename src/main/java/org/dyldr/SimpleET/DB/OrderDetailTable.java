package org.dyldr.SimpleET.DB;


import org.dyldr.SimpleET.Entity.OrderDetailEntity;
import org.dyldr.SimpleET.Entity.OrderEntity;
import org.dyldr.SimpleET.Entity.ProductEntity;
import org.springframework.data.repository.CrudRepository;

import java.util.List;

public interface OrderDetailTable  extends CrudRepository<OrderDetailEntity, Integer> {
    List<OrderDetailEntity> findByOrder(OrderEntity order);


}
