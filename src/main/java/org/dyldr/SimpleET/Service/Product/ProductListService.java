package org.dyldr.SimpleET.Service.Product;

import jakarta.transaction.Transactional;
import org.dyldr.SimpleET.DB.ProductTable;
import org.dyldr.SimpleET.DB.UserTable;
import org.dyldr.SimpleET.Entity.ProductEntity;

import org.dyldr.SimpleET.Entity.UserEntity;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.data.jpa.domain.Specification;
import org.springframework.stereotype.Service;

import java.util.List;

@Service
public class ProductListService {

    @Autowired
    private ProductTable prdTable;

    @Autowired
    UserTable userTable;

    public List<ProductEntity> searchBy(String name,
                                        String category,
                                        String sellerName,
                                        // Below are Additional Parameters
                                        double minPrice,
                                        double maxPrice,
                                        double maxExpressFee) {
        Specification<ProductEntity> spec = (root, query, cb) -> {
            return cb.and(
                    cb.like(root.get("name"), "%" + name + "%"),
                    cb.like(root.get("category"), "%" + category + "%"),
                    cb.like(root.get("seller").get("name"), "%" + sellerName + "%"),
                    cb.between(root.get("price"), minPrice, maxPrice),
                    cb.lessThanOrEqualTo(root.get("expressFee"), maxExpressFee)
            );
        };

        return prdTable.findAll(spec);
    }

    @Transactional
    public ProductEntity addProduct(
            String name,
            String description,
            double price,
            int stock,
            String category,
            String image,
            String seller,
            double expressFee,
            ProductEntity.Status status
    ) {
        ProductEntity product = new ProductEntity();
        product.setName(name);
        product.setDescription(description);
        product.setPrice(price);
        product.setStock(stock);
        product.setCategory(category);
        product.setImage(image);
        product.setExpressFee(expressFee);
        UserEntity sell = new UserEntity();
        sell.setId(seller);
        product.setSeller(sell);
        product.setStatus(status); // Default to off sale
        prdTable.save(product);
        return product;
    }

    @Transactional
    public boolean deleteProduct(String pid) {
        ProductEntity p = prdTable.findById(pid).orElse(null);
        if (p == null) {
            return false;
        }
        p.setStatus(ProductEntity.Status.DELETED);
        prdTable.save(p);
        return true;
    }

    @Transactional
    public boolean deleteProduct(String pid, String sellerId) {
        ProductEntity p = prdTable.findById(pid).orElse(null);
        if (p == null || !p.getSeller().getId().equals(sellerId)) {
            return false;
        }
        p.setStatus(ProductEntity.Status.DELETED);
        prdTable.save(p);
        return true;
    }

    public ProductEntity getProduct(String pid) {
        return prdTable.findById(pid).orElse(null);
    }

    @Transactional
    public ProductEntity updateProduct(String pid,
                                       String name,
                                       String description,
                                       double price,
                                       int stock,
                                       String category,
                                       String image,
                                       String seller,
                                       double expressFee,
                                        ProductEntity.Status status
    ) {
        ProductEntity product = new ProductEntity();
        product.setId(pid);
        product.setName(name);
        product.setDescription(description);
        product.setPrice(price);
        product.setStock(stock);
        product.setCategory(category);
        product.setImage(image);
        product.setExpressFee(expressFee);
        UserEntity sell = new UserEntity();
        sell.setId(seller);
        product.setSeller(sell);
        product.setStatus(status);
        prdTable.save(product);
        return product;
    }


}
