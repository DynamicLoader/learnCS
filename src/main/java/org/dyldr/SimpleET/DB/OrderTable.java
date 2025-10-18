package org.dyldr.SimpleET.DB;

import org.dyldr.SimpleET.Entity.OrderEntity;
import org.dyldr.SimpleET.Entity.ProductEntity;
import org.springframework.data.repository.CrudRepository;
import org.dyldr.SimpleET.Entity.UserEntity;

import java.util.List;

public interface OrderTable extends CrudRepository<OrderEntity, String> {

    public List<OrderEntity> findByCustomer(UserEntity customer);

}
