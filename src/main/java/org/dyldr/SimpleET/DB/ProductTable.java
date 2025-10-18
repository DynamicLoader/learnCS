package org.dyldr.SimpleET.DB;

import org.dyldr.SimpleET.Entity.ProductEntity;
import org.springframework.data.repository.CrudRepository;

import org.springframework.data.jpa.repository.JpaSpecificationExecutor;

import java.util.List;

public interface ProductTable extends CrudRepository<ProductEntity, String>, JpaSpecificationExecutor<ProductEntity> {

    public List<ProductEntity> findByName(String name);

    public List<ProductEntity> findByCategory(String category);

    // joined query
    public List<ProductEntity> findBySeller_Name(String sellerName);

//    public List<ProductEntity> findByNameOrCategoryOrSeller_Name(String name, String category, String sellerName);

//    public void DeleteById(String productName, String sellerId);

    public List<ProductEntity> findBySeller_Id(String sellerId);

//    public boolean existsById(String id);

//    public void deleteById(String id);

    public boolean existsByIdAndSeller_Id(String id, String sellerId);
    public void deleteByIdAndSeller_Id(String id, String sellerId);

}