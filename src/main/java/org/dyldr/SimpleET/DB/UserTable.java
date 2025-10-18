package org.dyldr.SimpleET.DB;

import org.springframework.data.repository.CrudRepository;
import org.dyldr.SimpleET.Entity.UserEntity;

import java.util.List;

public interface UserTable extends CrudRepository<UserEntity, String>{

//    public UserEntity findById(String id);
    public List<UserEntity> findByName(String name);

}



